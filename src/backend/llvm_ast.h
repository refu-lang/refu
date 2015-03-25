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
struct type;

struct LLVMOpaqueModule;
struct LLVMOpaqueTargetData;
struct LLVMOpaqueBuilder;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;
struct LLVMOpaqueBasicBlock;

struct llvm_traversal_ctx {
    struct LLVMOpaqueModule *mod;
    struct LLVMOpaqueBuilder *builder;
    struct LLVMOpaqueValue *current_value;
    struct LLVMOpaqueValue *current_function;
    struct LLVMOpaqueValue *current_function_return;
    struct LLVMOpaqueBasicBlock *current_block;
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

enum llvm_expression_compile_options {
    //! If the node is a simple elementary identifier return its value and not the Alloca
    RFLLVM_OPTION_IDENTIFIER_VALUE = 0x1,
};

/**
 * Compile an AST node expression to LLVMValueRef
 *
 * @param n        The ast node to compile
 * @param ctx      The llvm traversal context
 * @param options  [Optional] Options to provide for the compilations. For defaults
 *                 just give 0. Possible options are described at @ref llvm_expression_compile_options
 */
struct LLVMOpaqueValue *backend_llvm_expression_compile(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx,
                                                        int options);

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


void backend_llvm_ifexpr_compile(struct rir_branch *branch,
                                 struct llvm_traversal_ctx *ctx);

void backend_llvm_branch_compile(struct rir_branch *branch,
                                 struct llvm_traversal_ctx *ctx);

/**
 * Will typecast @a val if needed to a specific elementary type
 */
struct LLVMOpaqueValue *backend_llvm_cast_value_to_elementary_maybe(
    struct LLVMOpaqueValue  *val,
    const struct type *t,
    struct llvm_traversal_ctx *ctx);
#endif
