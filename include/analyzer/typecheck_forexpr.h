#ifndef LFR_TYPECHECK_FOREXPR_H
#define LFR_TYPECHECK_FOREXPR_H

struct ast_node;
struct analyzer_traversal_ctx;

enum traversal_cb_res typecheck_forexpr(struct ast_node *n,
                                        struct analyzer_traversal_ctx *ctx);

#endif
