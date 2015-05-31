#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H

#include <stdbool.h>

#include <Definitions/inline.h>
#include <Data_Structures/darray.h>
#include <types/type_decls.h>
#include <ir/rir_type_decls.h>
#include "llvm_types.h"

struct RFstring;

struct ast_node;
struct analyzer;
struct symbol_table;
struct symbol_table_record;
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
    struct {darray(struct LLVMOpaqueValue*);} values;
    //! Map from rir type to LLVM structs
    struct rir_types_map types_map;

    struct compiler_args *args;
    struct rir *rir;
    struct symbol_table *current_st;
};

bool bllvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root);
struct LLVMOpaqueModule *blvm_create_module(const struct ast_node *ast,
                                            struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueType *bllvm_elementary_to_type(enum elementary_type etype,
                                                struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueType *bllvm_type_from_type(const struct type *type,
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

static inline void llvm_traversal_ctx_add_param(struct llvm_traversal_ctx *ctx,
                                                struct LLVMOpaqueType *type)
{
    darray_append(ctx->params, type);
}

/**
 * Gets the values array from the llvm traversal ctx or NULL if
 * there are none
 */
i_INLINE_DECL struct LLVMOpaqueValue **llvm_traversal_ctx_get_values(struct llvm_traversal_ctx *ctx)
{
    return (darray_size(ctx->values) == 0) ? NULL : ctx->values.item;
}

i_INLINE_DECL unsigned llvm_traversal_ctx_get_values_count(struct llvm_traversal_ctx *ctx)
{
    return darray_size(ctx->values);
}

i_INLINE_DECL void llvm_traversal_ctx_reset_values(struct llvm_traversal_ctx *ctx)
{
    while (!darray_empty(ctx->values)) {
        (void)darray_pop(ctx->values);
    }
}

/* i_INLINE_DECL void llvm_traversal_ctx_add_value(struct llvm_traversal_ctx *ctx, */
static inline void llvm_traversal_ctx_add_value(struct llvm_traversal_ctx *ctx,
                                                struct LLVMOpaqueValue *value)
{
    darray_append(ctx->values, value);
}

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
struct LLVMOpaqueValue *bllvm_compile_expression(struct ast_node *n,
                                                 struct llvm_traversal_ctx *ctx,
                                                 int options);



void llvm_symbols_iterate_cb(struct symbol_table_record *rec,
                             struct llvm_traversal_ctx *ctx);
void bllvm_compile_block(const struct ast_node *block,
                         struct llvm_traversal_ctx *ctx);


void bllvm_compile_ifexpr(const struct ast_node *branch,
                          struct llvm_traversal_ctx *ctx);

void bllvm_compile_branch(const struct ast_node *branch,
                          struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *bllvm_compile_explicit_cast(const struct type *cast_type,
                                                    struct ast_node *args,
                                                    struct llvm_traversal_ctx *ctx);

/**
 * Will typecast @a val if needed to a specific elementary type
 */
struct LLVMOpaqueValue *bllvm_cast_value_to_elementary_maybe(
    struct LLVMOpaqueValue  *val,
    const struct type *t,
    struct llvm_traversal_ctx *ctx);

#endif
