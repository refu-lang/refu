#ifndef LFR_IR_PROCESS_H
#define LFR_IR_PROCESS_H

#include <stdbool.h>

struct ast_node;
struct rir_ctx;

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);
bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx);

#endif
