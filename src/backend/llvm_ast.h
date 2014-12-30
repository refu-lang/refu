#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H

#include <stdbool.h>

#include <Definitions/inline.h>
#include <Data_Structures/darray.h>

struct ast_node;
struct analyzer;
struct symbol_table;
struct compiler_args;

struct LLVMOpaqueModule;
struct LLVMOpaqueBuilder;

struct llvm_traversal_ctx {
    struct LLVMOpaqueModule *mod;
    struct LLVMOpaqueBuilder *builder;
    struct {darray(struct LLVMOpaqueType*);} params;

    struct analyzer *analyzer;
    struct compiler_args *args;
    struct symbol_table *current_st;
};

bool backend_llvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root);

/**
 * Gets the parameters array from the llvm traversal ctx or NULL if
 * there are none
 */
i_INLINE_DECL struct LLVMOpaqueType **llvm_traversal_ctx_get_params(struct llvm_traversal_ctx *ctx)
{
    return (darray_size(ctx->params) == 0) ? NULL : ctx->params.item;
}

i_INLINE_DECL unsigned llvm_traversal_ctx_get_param_count(struct llvm_traversal_ctx *ctx)
{
    return darray_size(ctx->params);
}
#endif
