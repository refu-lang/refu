#include "llvm_ast.h"

#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>
#include <Utils/sanity.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <compiler_args.h>

#include <ast/ast_utils.h>
#include <ast/function.h>
#include <ast/type.h>

#include <types/type_function.h>
#include <types/type_builtin.h>

#include <backend/llvm.h>

static LLVMTypeRef backend_llvm_builtin_to_type(enum builtin_type type)
{
    switch(type) {
        // LLVM does not differentiate between signed and unsigned
    case BUILTIN_INT:
    case BUILTIN_UINT:
        return LLVMIntType(32);// TODO: Think of how to represent size agnostic
    case BUILTIN_INT_8:
    case BUILTIN_UINT_8:
        return LLVMInt8Type();
    case BUILTIN_INT_16:
    case BUILTIN_UINT_16:
        return LLVMInt16Type();
    case BUILTIN_INT_32:
    case BUILTIN_UINT_32:
        return LLVMInt32Type();
    case BUILTIN_INT_64:
    case BUILTIN_UINT_64:
        return LLVMInt64Type();

    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Unsupported builtin type \""RF_STR_PF_FMT"\" "
                              "during LLVM conversion",
                              RF_STR_PF_ARG(type_builtin_get_str(type)));
        break;
    }
    return NULL;
}

static bool backend_llvm_typedesc_to_args(struct type_leaf *t,
                                          struct llvm_traversal_ctx *ctx)
{
    // TODO: only builtin types for now
    enum builtin_type btype;
    RF_ASSERT(t->type->category == TYPE_CATEGORY_BUILTIN,
              "Only dealing with builtin types at the moment in the backend");
    btype = type_builtin(t->type);

    LLVMTypeRef llvm_type = backend_llvm_builtin_to_type(btype);
    darray_append(ctx->params, llvm_type);
    return true;
}

static bool backend_llvm_function_body(struct ast_node *n,
                                       LLVMValueRef function,
                                       struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "function_entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    LLVMValueRef ret = LLVMConstInt(LLVMInt32Type(), 13, 0);
    LLVMBuildRet(ctx->builder, ret);
    return true;
}

static bool backend_llvm_function(struct ast_node *n,
                                  struct llvm_traversal_ctx *ctx)
{
    char *fnname;
    LLVMValueRef fn;
    bool at_first;
    struct ast_node *fndecl = ast_fnimpl_fndecl_get(n);
    const struct RFstring *fn_name = ast_fndecl_name_str(fndecl);
    struct ast_node *fn_args = ast_fndecl_args_get(fndecl);
    struct type *fn_type = symbol_table_lookup_type(ctx->current_st,
                                                    fn_name,
                                                    &at_first);
    if (RF_CRITICAL_TEST(!fn_type, "couldn't find function " RF_STR_PF_FMT
                         "in the symbol table.", RF_STR_PF_ARG(fn_name))) {
        return false;
    }

    // set the current symbol table
    ctx->current_st = ast_fndecl_symbol_table_get(fndecl);

    RFS_buffer_push();
    fnname = rf_string_cstr_from_buff(fn_name);

    // TODO: Properly implement this. For now it just takes all leaf types
    // of the argument and adds them to LLVM function args

    if (fn_args &&
        !type_for_each_leaf(type_function_get_argtype(fn_type),
                            (leaf_type_cb)backend_llvm_typedesc_to_args,
                            ctx)) {
        return false;
    }

    fn = LLVMAddFunction(ctx->mod, fnname,
                         LLVMFunctionType(LLVMInt32Type(),
                                          llvm_traversal_ctx_get_params(ctx),
                                          llvm_traversal_ctx_get_param_count(ctx),
                                          0)); // not variadic for now
    RFS_buffer_pop();
    return backend_llvm_function_body(ast_fnimpl_body_get(n), fn, ctx);
}

static bool backend_llvm_create_ir_ast_do(struct ast_node *n, void *user_arg)
{
    struct llvm_traversal_ctx *ctx = user_arg;
    switch(n->type) {
    case AST_ROOT:
        ctx->current_st = ast_root_symbol_table_get(n);
        ctx->mod = LLVMModuleCreateWithName(rf_string_data(compiler_args_get_output(ctx->args)));
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


i_INLINE_INS struct LLVMOpaqueType **llvm_traversal_ctx_get_params(struct llvm_traversal_ctx *ctx);
i_INLINE_INS unsigned llvm_traversal_ctx_get_param_count(struct llvm_traversal_ctx *ctx);
