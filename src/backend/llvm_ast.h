#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H

#include <stdbool.h>

#include <rflib/defs/inline.h>
#include <rflib/datastructs/darray.h>
#include <rflib/datastructs/strmap.h>

#include <types/type_decls.h>
#include "llvm_types.h"

struct RFstring;

struct ast_node;
struct ast_constant;
struct analyzer;
struct symbol_table;
struct symbol_table_record;
struct compiler_args;
struct analyzer;
struct type;
struct module;
struct rir;
struct rir_value;
struct rir_expression;

struct LLVMOpaqueModule;
struct LLVMOpaqueTargetData;
struct LLVMOpaqueBuilder;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;
struct LLVMOpaqueBasicBlock;

struct rirval_strmap {
    STRMAP_MEMBERS(void*);
};

struct llvm_traversal_ctx {
    struct module *mod;
    struct LLVMOpaqueModule *llvm_mod;
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

    //! Current rir function
    struct rir_fndef *current_rfn;
    struct compiler_args *args;
    //! Map from a rir value's id to llvm values
    struct rirval_strmap valmap;
};

bool bllvm_create_ir_ast(struct llvm_traversal_ctx *ctx, struct ast_node *root);
struct LLVMOpaqueModule *blvm_create_module(struct rir *rir,
                                            struct llvm_traversal_ctx *ctx,
                                            struct LLVMOpaqueModule *link_source);

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

static inline void llvm_traversal_ctx_prepend_param(struct llvm_traversal_ctx *ctx,
                                                    struct LLVMOpaqueType *type)
{
    darray_prepend(ctx->params, type);
}

struct rir *llvm_traversal_ctx_rir(struct llvm_traversal_ctx *ctx);

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

static inline void llvm_traversal_ctx_add_value(struct llvm_traversal_ctx *ctx,
                                                struct LLVMOpaqueValue *value)
{
    darray_append(ctx->values, value);
}

bool llvm_traversal_ctx_map_llvmval(struct llvm_traversal_ctx *ctx,
                                    const struct rir_value *rv,
                                    struct LLVMOpaqueValue *lv);
bool llvm_traversal_ctx_map_llvmblock(struct llvm_traversal_ctx *ctx,
                                      const struct rir_value *rv,
                                      struct LLVMOpaqueBasicBlock *lb);
void llvm_traversal_ctx_reset_valmap(struct llvm_traversal_ctx *ctx);

enum llvm_expression_compile_options {
    //! If the node is a simple elementary identifier return its value and not the Alloca
    RFLLVM_OPTION_IDENTIFIER_VALUE = 0x1,
};

struct LLVMOpaqueValue *bllvm_compile_rirexpr(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx
);



void llvm_symbols_iterate_cb(struct symbol_table_record *rec,
                             struct llvm_traversal_ctx *ctx);
void bllvm_compile_block(const struct ast_node *block,
                         struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *bllvm_compile_constant(const struct ast_constant *n,
                                               struct rir_type *type,
                                               struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *bllvm_compile_literal(const struct RFstring *lit,
                                              struct llvm_traversal_ctx *ctx);


void bllvm_compile_ifexpr(const struct ast_node *branch,
                          struct llvm_traversal_ctx *ctx);

void bllvm_compile_branch(const struct ast_node *branch,
                          struct llvm_traversal_ctx *ctx);

/**
 * Will typecast @a val if needed to a specific elementary type
 */
struct LLVMOpaqueValue *bllvm_cast_value_to_elementary_maybe(
    struct LLVMOpaqueValue  *val,
    const struct type *t,
    struct llvm_traversal_ctx *ctx);

#endif
