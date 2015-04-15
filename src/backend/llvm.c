#include <backend/llvm.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <String/rf_str_core.h>
#include <System/rf_system.h>
#include <Persistent/buffers.h>

#include <info/info.h>
#include <analyzer/analyzer.h>
#include <compiler_args.h>
#include <ir/elements.h>

#include "llvm_ast.h"


static inline void llvm_traversal_ctx_init(struct llvm_traversal_ctx *ctx,
                                           struct rir *rir,
                                           struct compiler_args *args)
{
    ctx->mod = NULL;
    ctx->current_st = NULL;
    ctx->current_function = NULL;
    ctx->rir = rir;
    ctx->args = args;
    ctx->builder = LLVMCreateBuilder();
    darray_init(ctx->params);
    darray_init(ctx->values);
}

static inline void llvm_traversal_ctx_deinit(struct llvm_traversal_ctx *ctx)
{
    darray_free(ctx->params);
    darray_free(ctx->values);
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->mod);
    LLVMDisposeTargetData(ctx->target_data);
}

static bool backend_llvm_ir_generate(struct rir_module *module, struct rir *rir,
                                     struct compiler_args *args)
{
    struct llvm_traversal_ctx ctx;
    struct LLVMOpaqueModule *llvm_module;
    bool ret = false;
    char *error = NULL; // Used to retrieve messages from functions

    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeNativeTarget();

    llvm_traversal_ctx_init(&ctx, rir, args);
    llvm_module = backend_llvm_create_module(module, &ctx);
    if (!llvm_module) {
        ERROR("Failed to form the LLVM IR ast");
        goto end;
    }

    // verify module and create code
    LLVMVerifyModule(llvm_module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors

    RFS_push();
    struct RFstring *temp_string;
    RFS(&temp_string, RF_STR_PF_FMT".ll",
        RF_STR_PF_ARG(compiler_args_get_output(args)));

    const char *s = rf_string_cstr_from_buff(temp_string);
    if (0 != LLVMPrintModuleToFile(ctx.mod, s, &error)) {
        ERROR("LLVM-error: %s", error);
        LLVMDisposeMessage(error);
        goto end;
    }

    ret = true;
end:
    RFS_pop();
    llvm_traversal_ctx_deinit(&ctx);
    LLVMShutdown();
    return ret;
}

static bool transformation_step_do(struct compiler_args *args,
                                   const char *executable,
                                   const char *insuff,
                                   const char *outsuff)
{
    int rc;
    FILE *proc;
    struct RFstring *inname;
    struct RFstring *cmd;
    const struct RFstring* output = compiler_args_get_output(args);
    bool ret = true;
    RFS_push();

    RFS(&inname, RF_STR_PF_FMT".%s",
        RF_STR_PF_ARG(output), insuff);
    RFS(&cmd,
        "%s "RF_STR_PF_FMT" -o "RF_STR_PF_FMT".%s",
        executable, RF_STR_PF_ARG(inname),
        RF_STR_PF_ARG(output), outsuff);
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
    RFS_pop();
    return ret;
}

static bool backend_llvm_ir_to_asm(struct compiler_args *args)
{
    return transformation_step_do(args, "llc", "ll", "s");
}

static bool backend_asm_to_exec(struct compiler_args *args)
{
    return transformation_step_do(args, "gcc", "s", "exe");
}

bool backend_llvm_generate(struct rir_module *module, struct rir *r,
                           struct compiler_args *args)
{

    if (!backend_llvm_ir_generate(module, r, args)) {
        return false;
    }

    // now it should be okay to free the module
    rir_module_destroy(module);

    if (!backend_llvm_ir_to_asm(args)) {
        ERROR("Failed to generate assembly from LLVM IR code");
        return false;
    }

    if (!backend_asm_to_exec(args)) {
        ERROR("Failed to generate executable from assembly machine code");
        return false;
    }

    return true;
}
