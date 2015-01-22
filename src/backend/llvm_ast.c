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
#include <ast/block.h>
#include <ast/returnstmt.h>
#include <ast/constant_num.h>

#include <types/type_function.h>
#include <types/type_elementary.h>

#include <backend/llvm.h>

static LLVMTypeRef backend_llvm_elementary_to_type(enum elementary_type type)
{
    switch(type) {
        // LLVM does not differentiate between signed and unsigned
    case ELEMENTARY_TYPE_INT:
    case ELEMENTARY_TYPE_UINT:
        return LLVMIntType(32);// TODO: Think of how to represent size agnostic
    case ELEMENTARY_TYPE_INT_8:
    case ELEMENTARY_TYPE_UINT_8:
        return LLVMInt8Type();
    case ELEMENTARY_TYPE_INT_16:
    case ELEMENTARY_TYPE_UINT_16:
        return LLVMInt16Type();
    case ELEMENTARY_TYPE_INT_32:
    case ELEMENTARY_TYPE_UINT_32:
        return LLVMInt32Type();
    case ELEMENTARY_TYPE_INT_64:
    case ELEMENTARY_TYPE_UINT_64:
        return LLVMInt64Type();

    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Unsupported elementary type \""RF_STR_PF_FMT"\" "
                              "during LLVM conversion",
                              RF_STR_PF_ARG(type_elementary_get_str(type)));
        break;
    }
    return NULL;
}

static bool backend_llvm_typedesc_to_args(struct type_leaf *t,
                                          struct llvm_traversal_ctx *ctx)
{
    // TODO: only elementary types for now
    enum elementary_type etype;
    RF_ASSERT(t->type->category == TYPE_CATEGORY_ELEMENTARY,
              "Only dealing with elementary types at the moment in the backend");
    etype = type_elementary(t->type);

    LLVMTypeRef llvm_type = backend_llvm_elementary_to_type(etype);
    darray_append(ctx->params, llvm_type);
    return true;
}

static bool backend_llvm_function_body(struct ast_node *n,
                                       LLVMValueRef function,
                                       struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "function_entry");
    struct ast_node *retstmt;
    struct ast_node *retexpr;
    uint64_t val;
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    // TODO: make this proper in the future. For now just create the return value
    // and only if it's a const number
    retstmt = ast_block_valueexpr_get(n);
    retexpr = ast_returnstmt_expr_get(retstmt);
    RF_ASSERT(ast_node_type(retexpr) == AST_CONSTANT_NUMBER,
    "Only constant number supported in returns for now");
    if (!ast_constantnum_get_integer(retexpr, &val)) {
        RF_ERROR("Failed to convert a constant num node to number");
        return false;
    }
    LLVMValueRef ret = LLVMConstInt(LLVMInt32Type(), val, 0);
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
