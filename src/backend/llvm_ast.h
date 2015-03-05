#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H

#include <stdbool.h>

#include <Definitions/inline.h>
#include <Data_Structures/darray.h>

struct RFstring;

struct ast_node;
struct analyzer;
struct symbol_table;
struct compiler_args;
struct rir;
struct rir_module;
struct rir_type;
struct rir_basic_block;
struct rir_branch;

struct LLVMOpaqueModule;
struct LLVMOpaqueTargetData;
struct LLVMOpaqueBuilder;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

struct llvm_traversal_ctx {
    struct LLVMOpaqueModule *mod;
    struct LLVMOpaqueBuilder *builder;
    struct LLVMOpaqueValue *current_value;
    struct LLVMOpaqueValue *current_function;
    struct LLVMOpaqueTargetData *target_data;
    struct {darray(struct LLVMOpaqueType*);} params;

    struct compiler_args *args;
    struct rir *rir;
    struct symbol_table *current_st;
};

bool backend_llvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root);
struct LLVMOpaqueModule *backend_llvm_create_module(struct rir_module *mod,
                                                    struct llvm_traversal_ctx *ctx);
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

i_INLINE_DECL void llvm_traversal_ctx_reset_params(struct llvm_traversal_ctx *ctx)
{
    while (!darray_empty(ctx->params)) {
        (void)darray_pop(ctx->params);
    }
}

void llvm_traversal_ctx_add_param(struct llvm_traversal_ctx *ctx,
                                  struct LLVMOpaqueType *type);

/**
 * Compile an AST node expression to LLVMValueRef
 */
struct LLVMOpaqueValue *backend_llvm_expression_compile(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx);

/**
 * Compile a type declaration
 *
 * @param name        Provide the name of the type to create
 * @param type        [optional] Can provide the rir_type to declare here.
 *                    IF it's NULL then the type is searched for in the rir types list
 * @param ctx         The llvm traversal context
 */
void backend_llvm_compile_typedecl(const struct RFstring *name,
                                   struct rir_type *type,
                                   struct llvm_traversal_ctx *ctx);


void backend_llvm_compile_basic_block(struct rir_basic_block *block,
                                      struct llvm_traversal_ctx *ctx);


struct LLVMOpaqueValue *backend_llvm_ifexpr_compile(struct rir_branch *branch,
                                                    struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *backend_llvm_branch_compile(struct rir_branch *branch,
                                                    struct llvm_traversal_ctx *ctx);
#endif
