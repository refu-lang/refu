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
#include <compiler_args.h>


static bool backend_llvm_ir_generate(struct ast_node *ast, struct compiler_args *args)
{
    char *error = NULL; // Used to retrieve messages from functions
    LLVMLinkInJIT();
    LLVMInitializeNativeTarget();
    LLVMModuleRef mod = LLVMModuleCreateWithName(rf_string_data(args->output));

    LLVMValueRef main_fn = LLVMAddFunction(mod, "main", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_fn, "main_entry");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, entry);
    LLVMValueRef main_ret = LLVMConstInt(LLVMInt32Type(), 15, 0);
    LLVMBuildRet(builder, main_ret);

    LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors

    RFS_buffer_push();
    const struct RFstring *temp_string = RFS_(RF_STR_PF_FMT".ll",
                                              RF_STR_PF_ARG(args->output));

    const char *s = rf_string_cstr_from_buff(temp_string);
    if (0 != LLVMPrintModuleToFile(mod, s, &error)) {
        ERROR("LLVM-error: %s", error);
        LLVMDisposeMessage(error);
        RFS_buffer_pop();
        return false;
    }
    RFS_buffer_pop();

    LLVMDisposeBuilder(builder);
    return true;
}

static bool transformation_step_do(struct compiler_args *args,
                                   const char *executable,
                                   const char *insuff,
                                   const char *outsuff)
{
    int rc;
    FILE *proc;
    bool ret = true;
    RFS_buffer_push();

    struct RFstring *inname = RFS_(RF_STR_PF_FMT".%s",
                                   RF_STR_PF_ARG(args->output), insuff);

    fflush(stdout);
    proc = rf_popen(RFS_("%s "RF_STR_PF_FMT" -o "RF_STR_PF_FMT".%s",
                         executable, RF_STR_PF_ARG(inname),
                         RF_STR_PF_ARG(args->output), outsuff),
                    "r");

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
    RFS_buffer_pop();
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

bool backend_llvm_generate(struct ast_node *ast, struct compiler_args *args)
{

    if (!backend_llvm_ir_generate(ast, args)) {
        return false;
    }

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
