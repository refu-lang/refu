#ifndef LFR_AST_RETURNSTMT_H
#define LFR_AST_RETURNSTMT_H

#include <ast/ast.h>

struct ast_node *ast_returnstmt_create(const struct inplocation_mark *start,
                                       const struct inplocation_mark *end,
                                       struct ast_node *expr);

i_INLINE_DECL struct ast_node *ast_returnstmt_expr_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_RETURN_STATEMENT);
    return n->returnstmt.expr;
}
#endif
