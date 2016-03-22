#include <ast/arr.h>
#include <ast/ast.h>

struct ast_node *ast_arrspec_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct arr_ast_nodes *dimensions
)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_ARRAY_SPEC, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_copy_children(ret, dimensions);
    return ret;
}

i_INLINE_INS unsigned int ast_arrspec_dimensions_num(struct ast_node *n);
