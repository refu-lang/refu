#ifndef LFR_TYPECHECK_MATCHEXPR_H
#define LFR_TYPECHECK_MATCHEXPR_H

#include <utils/traversal.h>

struct ast_node;
struct analyzer_traversal_ctx;

enum traversal_cb_res typecheck_matchcase(struct ast_node *n,
                                          struct analyzer_traversal_ctx* ctx);
enum traversal_cb_res typecheck_matchexpr(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx);

#endif
