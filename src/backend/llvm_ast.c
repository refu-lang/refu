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

static bool backend_llvm_typedesc_to_args(struct ast_node *desc,
                                          struct llvm_traversal_ctx *ctx)
{

    if (desc->type == AST_TYPE_OPERATOR) {
        //TODO: For now only commas here, no sum operation on types yet
        RF_ASSERT(ast_typeop_op(desc) == TYPEOP_PRODUCT,
                  "Only product types supported for now in the backend");
    } else if (desc->type == AST_TYPE_DESCRIPTION) {
        struct ast_node *left = ast_typedesc_left(desc);
        struct symbol_table_record *rec;
        bool at_first;
        struct type* type;
        // TODO: no complex types for now
        RF_ASSERT(left->type == AST_IDENTIFIER,
                  "No complex types in the backend for now");

        rec = symbol_table_lookup_record(ctx->current_st, ast_identifier_str(left), &at_first);
        RF_ASSERT_OR_CRITICAL(rec != NULL,
                              "Symbol table lookup failed. Should never happen at this stage");

        type = symbol_table_record_type(rec);

        // TODO: only builtin types for now
        enum builtin_type btype;
        RF_ASSERT(type->category == TYPE_CATEGORY_BUILTIN || type->category == TYPE_CATEGORY_LEAF,
                  "Only dealing with builtin types at the moment in the backend");
        if (type->category == TYPE_CATEGORY_LEAF) {
            RF_ASSERT(type->leaf.type->category == TYPE_CATEGORY_BUILTIN,
                      "Only dealing with builtin types at the moment in the backend");
            btype = type_builtin(type->leaf.type);
        } else {
            btype = type_builtin(type);
        }

        LLVMTypeRef llvm_type = backend_llvm_builtin_to_type(btype);
        darray_append(ctx->params, llvm_type);
    }

    // ignore all other node types under the type description

    return true;
}

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

    // set the current symbol table
    ctx->current_st = ast_fndecl_symbol_table_get(fndecl);

    RFS_buffer_push();
    fnname = rf_string_cstr_from_buff(ast_fndecl_name_str(fndecl));

    //for now totally ignore function's types, all accept and return an int
    if (!ast_pre_traverse_tree(ast_fndecl_args_get(fndecl),
                               (ast_node_cb)backend_llvm_typedesc_to_args,
                               ctx)) {
        return false;
    }

    fn = LLVMAddFunction(ctx->mod, fnname,
                         LLVMFunctionType(LLVMInt32Type(), ctx->params.item, 1, 0));
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
