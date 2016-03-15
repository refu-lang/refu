#ifndef LFR_IR_PROCESS_H
#define LFR_IR_PROCESS_H

#include <stdbool.h>

#include <rflib/defs/inline.h>
#include <rflib/utils/log.h>

#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_utils.h>

struct ast_node;
struct rir_ctx;

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);

/**
 * Process an ast node and return the generated rir object
 */
i_INLINE_DECL struct rir_object *rir_process_ast_node_getobj(
    const struct ast_node *n,
    struct rir_ctx *ctx
)
{
    if (!rir_process_ast_node(n, ctx)) {
        return NULL;
    }
    return ctx->returned_obj;
}

/**
 * Process an ast node and return its rir value
 */
i_INLINE_DECL struct rir_value *rir_process_ast_node_getval(
    const struct ast_node *n,
    struct rir_ctx *ctx
)
{
    struct rir_object *obj = rir_process_ast_node_getobj(n, ctx);
    return obj ? rir_object_value(obj) : NULL;
}

/**
 * Process an ast node and get the value of the given expression. May perform read.
 * Uses @ref rir_getread_val()
 */
i_INLINE_DECL const struct rir_value *rir_process_ast_node_getreadval(
    const struct ast_node *n,
    struct rir_ctx *ctx
)
{
    struct rir_object *obj = rir_process_ast_node_getobj(n, ctx);
    RF_ASSERT(obj &&
              (obj->category == RIR_OBJ_GLOBAL ||
               obj->category == RIR_OBJ_EXPRESSION ||
               obj->category == RIR_OBJ_VARIABLE),
              "At this point either a rir global or a rir expression"
              "object should have been generated"
    );
    return obj->category == RIR_OBJ_EXPRESSION
        ? rirctx_getread_exprval(&obj->expr, ctx)
        : rir_object_value(obj);
}

bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx);
bool rir_match_st_populate_allocas(const struct ast_node *mcase,
                                   struct rir_object *matched_rir_obj,
                                   struct rir_ctx *ctx);
#endif
