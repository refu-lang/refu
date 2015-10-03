#ifndef LFR_IR_PROCESS_H
#define LFR_IR_PROCESS_H

#include <stdbool.h>
#include <Definitions/inline.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_utils.h>
#include <Utils/log.h>

struct ast_node;
struct rir_ctx;

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);

/**
 * Process an ast node and return the generated rir expression. Will return
 * NULL if an expression was not generated
 */
i_INLINE_DECL struct rir_expression *rir_process_ast_node_getexpr(const struct ast_node *n,
                                                                  struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(n, ctx)) {
        return NULL;
    }
    struct rir_object *obj = ctx->returned_obj;
    if (!obj || obj->category != RIR_OBJ_EXPRESSION) {
        return NULL;
    }
    return &obj->expr;
}

/**
 * Process an ast node and return its rir value
 */
i_INLINE_DECL struct rir_value *rir_process_ast_node_getval(const struct ast_node *n,
                                                           struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(n, ctx)) {
        return NULL;
    }
    return rir_ctx_lastval_get(ctx);
}

/**
 * Process an ast node and get the value of the given expression. May perform read.
 * Uses @ref rir_getread_val()
 */
i_INLINE_DECL const struct rir_value *rir_process_ast_node_getreadval(const struct ast_node *n,
                                                                      struct rir_ctx *ctx)
{
    struct rir_expression *expr = rir_process_ast_node_getexpr(n, ctx);
    if (!expr) {
        RF_ERROR("Failed to create a rir expression");
        return NULL;
    }
    return rir_getread_val(expr, ctx);
}

bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx);
bool rir_match_st_populate_allocas(const struct ast_node *mcase,
                                   struct rir_object *matched_rir_obj,
                                   struct rir_ctx *ctx);
#endif
