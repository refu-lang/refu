#ifndef LFR_TYPECHECK_TYPECLASS_H
#define LFR_TYPECHECK_TYPECLASS_H


struct ast_node;
struct analyzer_traversal_ctx;

enum traversal_cb_res typecheck_typeinstance(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);


#endif
