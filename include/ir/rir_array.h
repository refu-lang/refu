#ifndef LFR_IR_RIR_ARRAY_H
#define LFR_IR_RIR_ARRAY_H

#include <stdbool.h>

struct rirtostr_ctx;
struct rir_expression;
struct rir_ctx;
struct ast_node;

bool rir_process_bracketlist(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_fixedarr_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);

#endif
