#ifndef LFR_TYPECHECK_TYPECLASS_H
#define LFR_TYPECHECK_TYPECLASS_H

#include <stdbool.h>

struct ast_node;
struct analyzer_traversal_ctx;

struct typeclass_ctx {
    struct type *instantiated_type;
};


bool typeclass_ctx_init(struct typeclass_ctx *ctx, struct type *t);


enum traversal_cb_res typecheck_typeclass(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);

enum traversal_cb_res typecheck_typeinstance(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);


#endif
