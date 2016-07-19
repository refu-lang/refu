#include <ast/ast.h>
#include <ast/forexpr.h>

struct ast_node *ast_forexpr_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *loopvar,
    struct ast_node *iterable,
    struct ast_node *body)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_FOR_EXPRESSION, start, end);
    if (!ret) {
        return NULL;
    }

    ast_node_register_child(ret, loopvar, forexpr.loopvar);
    ast_node_register_child(ret, iterable, forexpr.iterable);
    ast_node_register_child(ret, body, forexpr.body);

    return ret;
}
