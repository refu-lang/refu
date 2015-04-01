#include "typecheck_matchexpr.h"

#include <ast/ast.h>
#include <ast/matchexpr.h>

enum traversal_cb_res typecheck_matchexpr(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx)
{
    
    const struct type *t = ast_matchexpr_identifier(n)->expression_type;
    (void)t;
    (void)ctx;
    return TRAVERSAL_CB_OK;
}
