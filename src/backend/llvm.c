#include <backend/llvm.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <rfbase/string/core.h>
#include <rfbase/system/system.h>
#include <rfbase/persistent/buffers.h>

#include <info/info.h>
#include <analyzer/analyzer.h>
#include <compiler_args.h>
#include <front_ctx.h>
#include <module.h>
#include <utils/common_strings.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

static void bllvm_diagnostic_handler(struct LLVMOpaqueDiagnosticInfo *di, void *u)
{
    char *str = LLVMGetDiagInfoDescription(di);
    printf("[LLVM-error]:%s.", str);
    fflush(stdout);
    LLVMDisposeMessage(str);
}

static inline void llvm_traversal_ctx_init(
    struct llvm_traversal_ctx *ctx,
    struct compiler_args *args)
{
    ctx->mod = NULL;
    ctx->current_function = NULL;
    ctx->args = args;
    ctx->llvm_context = LLVMContextCreate();
    LLVMContextSetDiagnosticHandler(
        ctx->llvm_context,
        bllvm_diagnostic_handler,
        NULL
    );

    ctx->builder = LLVMCreateBuilderInContext(ctx->llvm_context);

    darray_init(ctx->params);
    darray_init(ctx->values);
    rir_types_map_init(&ctx->types_map);
    strmap_init(&ctx->valmap);
}

static inline void llvm_traversal_ctx_deinit(struct llvm_traversal_ctx *ctx)
{
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->llvm_mod);
    LLVMContextDispose(ctx->llvm_context);
}

static inline void llvm_traversal_ctx_set_singlepass(struct llvm_traversal_ctx *ctx,
                                                     struct module *m)
{
    ctx->mod = m;
    ctx->llvm_mod = NULL;
    ctx->current_function = NULL;;
    darray_init(ctx->params);
    darray_init(ctx->values);
    rir_types_map_init(&ctx->types_map);
    strmap_init(&ctx->valmap);
}

static inline void llvm_traversal_ctx_reset_singlepass(struct llvm_traversal_ctx *ctx)
{
    ctx->mod = NULL;
    LLVMDisposeTargetData(ctx->target_data);
    rir_types_map_deinit(&ctx->types_map);
    darray_free(ctx->params);
    darray_free(ctx->values);
    strmap_clear(&ctx->valmap);
}

static bool bllvm_ir_generate(struct modules_arr *modules, struct compiler_args *args)
{
    struct llvm_traversal_ctx ctx;
    struct LLVMOpaqueModule *llvm_module;
    struct LLVMOpaqueModule *stdlib_module = NULL;
    bool ret = false;
    char *error = NULL; // Used to retrieve messages from functions

    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeNativeTarget();

    struct module **mod;
    llvm_traversal_ctx_init(&ctx, args);
    darray_foreach(mod, *modules) {
        llvm_traversal_ctx_set_singlepass(&ctx, *mod);
        llvm_module = blvm_create_module((*mod)->rir, &ctx, stdlib_module);
        if (!llvm_module) {
            ERROR("Failed to form the LLVM IR ast");
            llvm_traversal_ctx_reset_singlepass(&ctx);
            goto end;
        }

        // if this was stdlib mark it
        if (rf_string_equal(module_name(*mod), &g_str_stdlib)) {
            RF_ASSERT(!stdlib_module, "Two modules with the stdlib name");
            stdlib_module = llvm_module;
        }

        // verify each module
        if (LLVMVerifyModule(llvm_module, LLVMPrintMessageAction, &error) == 1) {
            bllvm_error("Could not verify LLVM module", &error);
            llvm_traversal_ctx_reset_singlepass(&ctx);
            goto end;
        }
        bllvm_error_dispose(&error);

        llvm_traversal_ctx_reset_singlepass(&ctx);
    }

    RFS_PUSH();
    struct RFstring *temp_s = RFS_NT_OR_DIE(
        RFS_PF".ll",
        RFS_PA(compiler_args_get_executable_name(args)));
    if (0 != LLVMPrintModuleToFile(llvm_module, rf_string_data(temp_s), &error)) {
        bllvm_error("Could not output LLVM module to file", &error);
        goto end_pop_rfs;
    }
    bllvm_error_dispose(&error);
    if (stdlib_module) {
        LLVMDisposeModule(stdlib_module);
    }
    llvm_traversal_ctx_deinit(&ctx);
    ret = true;

end_pop_rfs:
    RFS_POP();
end:
    LLVMShutdown();
    return ret;
}

static bool transformation_step_do(struct compiler_args *args,
                                   const char *executable,
                                   const char *insuff,
                                   const char *outsuff,
                                   const char *extra)
{
    int rc;
    FILE *proc;
    struct RFstring *inname;
    struct RFstring *cmd;
    const struct RFstring* output = compiler_args_get_executable_name(args);
    bool ret = true;
    RFS_PUSH();

    inname = RFS(RFS_PF".%s", RFS_PA(output), insuff);
    cmd = RFS(
        "%s "RFS_PF" %s -o "RFS_PF".%s",
        executable,
        RFS_PA(inname),
        extra ? extra : "",
        RFS_PA(output),
        outsuff);
    proc = rf_popen(cmd, "r");

    if (!proc) {
        ret = false;
        goto end;
    }

    rc = rf_pclose(proc);
    if (0 != rc) {
        ERROR("%s failed with error code: %d", executable, rc);
        ret = false;
        goto end;
    }

    // delete no longer needed input name file
    rf_system_delete_file(inname);
    fflush(stdout);
end:
    RFS_POP();
    return ret;
}

static bool bllvm_ir_to_asm(struct compiler_args *args)
{
    return transformation_step_do(args, "llc", "ll", "s", NULL);
}

static bool backend_asm_to_exec(struct compiler_args *args)
{
    return transformation_step_do(args, "gcc", "s", "exe", "-L"RF_LANG_CORE_ROOT"/build/rfbase/ -lrfbase -static");
}

bool bllvm_generate(struct modules_arr *modules, struct compiler_args *args)
{

    if (!bllvm_ir_generate(modules, args)) {
        return false;
    }

    if (!bllvm_ir_to_asm(args)) {
        ERROR("Failed to generate assembly from LLVM IR code");
        return false;
    }

    if (!backend_asm_to_exec(args)) {
        ERROR("Failed to generate executable from assembly machine code");
        return false;
    }

    return true;
}
