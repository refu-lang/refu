#include "llvm_ast.h"

#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <compiler_args.h>

#include <ast/ast_utils.h>
#include <ast/function.h>

#include <backend/llvm.h>

static bool backend_llvm_function_body(struct ast_node *n,
                                       LLVMValueRef function,
                                       struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "function_entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    LLVMValueRef ret = LLVMConstInt(LLVMInt32Type(), 15, 0);
    LLVMBuildRet(ctx->builder, ret);
    return true;
}

static bool backend_llvm_function(struct ast_node *n,
                                  struct llvm_traversal_ctx *ctx)
{
    char *fnname;
    LLVMValueRef fn;
    struct ast_node *fndecl = ast_fnimpl_fndecl_get(n);
    LLVMTypeRef args[] = { LLVMInt32Type() };

    RFS_buffer_push();
    fnname = rf_string_cstr_from_buff(ast_fndecl_name_str(fndecl));
    //for now totally ignore function's types, all accept and return an int
    fn = LLVMAddFunction(ctx->mod, fnname,
                         LLVMFunctionType(LLVMInt32Type(), args, 1, 0));
    RFS_buffer_pop();
    return backend_llvm_function_body(ast_fnimpl_body_get(n), fn, ctx);
}

static bool backend_llvm_create_ir_ast_do(struct ast_node *n, void *user_arg)
{
    struct llvm_traversal_ctx *ctx = user_arg;
    switch(n->type) {
    case AST_ROOT:
        ctx->mod = LLVMModuleCreateWithName(rf_string_data(ctx->args->output));
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        return backend_llvm_function(n, ctx);
    default:
        // do nothing
        break;
    }

    return true;
}


static bool end_visit_do(struct ast_node *n, void *user_arg)
{
    // nothing for now. IF indeed nothing is needed then use ast_pre_traverse_tree()
    return true;
}

bool backend_llvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root)
{
    return ast_traverse_tree(
        root,
        backend_llvm_create_ir_ast_do,
        ctx,
        end_visit_do, //POST callback
        NULL); //POST USER ARG

}
