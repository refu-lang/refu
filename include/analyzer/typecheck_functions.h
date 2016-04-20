#ifndef LFR_TYPECHECK_FUNCTIONS_H
#define LFR_TYPECHECK_FUNCTIONS_H

#include <utils/traversal.h>

struct type;
struct ast_node;
struct analyzer_traversal_ctx;

enum traversal_cb_res typecheck_function_call(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);

enum traversal_cb_res typecheck_fndecl(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);

#endif
