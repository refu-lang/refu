#ifndef LFR_IR_PROCESS_H
#define LFR_IR_PROCESS_H

#include <stdbool.h>
#include <Definitions/inline.h>
#include <ir/rir.h>

struct ast_node;
struct rir_ctx;

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);
i_INLINE_DECL struct rir_value *rir_processret_ast_node(const struct ast_node *n,
                                                        struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(n, ctx)) {
        return NULL;
    }
    return rir_ctx_lastval_get(ctx);
}

bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx);

#endif
