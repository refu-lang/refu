#ifndef LFR_TYPECHECK_FOREXPR_H
#define LFR_TYPECHECK_FOREXPR_H

#include <stdbool.h>

struct ast_node;
struct analyzer_traversal_ctx;

bool typecheck_forexpr_descending(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx);

enum traversal_cb_res typecheck_forexpr_ascending(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx);

#endif
