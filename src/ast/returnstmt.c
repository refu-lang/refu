#include <ast/returnstmt.h>

struct ast_node *ast_returnstmt_create(struct inplocation_mark *start,
                                       struct inplocation_mark *end,
                                       struct ast_node *expr)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_RETURN_STATEMENT, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->returnstmt.expr = expr;

    return ret;
}

