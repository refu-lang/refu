#include <ast/arrayref.h>

struct ast_node *ast_arrayref_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *expr)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_ARRAY_REFERENCE, start, end);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ast_node_register_child(ret, name, arrayref.name);
    ast_node_register_child(ret, expr, arrayref.expr);

    return ret;
}
