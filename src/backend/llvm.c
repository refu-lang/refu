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
#include <ir/rir.h>
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
    rir_types_map_init(&ctx->types_map);
}

static inline void llvm_traversal_ctx_deinit(struct llvm_traversal_ctx *ctx)
{
    rir_types_map_deinit(&ctx->types_map);
    darray_free(ctx->params);
    darray_free(ctx->values);
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->mod);
    LLVMDisposeTargetData(ctx->target_data);
}

static bool bllvm_ir_generate(struct rir *rir, struct compiler_args *args)
{
    struct llvm_traversal_ctx ctx;
    struct LLVMOpaqueModule *llvm_module;
    bool ret = false;
    char *error = NULL; // Used to retrieve messages from functions

    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeNativeTarget();

    llvm_traversal_ctx_init(&ctx, rir, args);
    llvm_module = blvm_create_module(rir->root, &ctx);
    if (!llvm_module) {
        ERROR("Failed to form the LLVM IR ast");
        goto end;
    }

    // verify module and create code
    LLVMVerifyModule(llvm_module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors

    RFS_PUSH();
    
    struct RFstring *temp_s = RFS_NT_OR_DIE(
        RF_STR_PF_FMT".ll",
        RF_STR_PF_ARG(compiler_args_get_executable_name(args)));
    if (0 != LLVMPrintModuleToFile(ctx.mod, rf_string_data(temp_s), &error)) {
        ERROR("LLVM-error: %s", error);
        LLVMDisposeMessage(error);
        goto end;
    }

    ret = true;
end:
    RFS_POP();
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
    const struct RFstring* output = compiler_args_get_executable_name(args);
    bool ret = true;
    RFS_PUSH();

    inname = RFS(RF_STR_PF_FMT".%s", RF_STR_PF_ARG(output), insuff);
    cmd = RFS(
        "%s "RF_STR_PF_FMT" -o "RF_STR_PF_FMT".%s",
        executable,
        RF_STR_PF_ARG(inname),
        RF_STR_PF_ARG(output),
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
    return transformation_step_do(args, "llc", "ll", "s");
}

static bool backend_asm_to_exec(struct compiler_args *args)
{
    return transformation_step_do(args, "gcc", "s", "exe");
}

bool bllvm_generate(struct rir *r, struct compiler_args *args)
{

    if (!bllvm_ir_generate(r, args)) {
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
