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


struct ast_node *ast_bracketlist_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *args
)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_BRACKET_LIST, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    if (args) {
        ast_node_add_child(ret, args);
    }
    return ret;
}

static bool populate_bracketlist_cb(struct ast_node *n, struct arr_ast_nodes *members)
{
    darray_push(*members, n);
    return true;
}

struct arr_ast_nodes *ast_bracketlist_members(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_BRACKET_LIST);
    if (n->state <= AST_NODE_STATE_ANALYZER_PASS1) {
        darray_init(n->bracketlist.members);
        ast_bracketlist_foreach_member(
            n,
            (exprlist_cb)populate_bracketlist_cb,
            &n->bracketlist.members
        );
        n->state = AST_NODE_STATE_TYPECHECK_1;
    }
    return &n->bracketlist.members;
}

i_INLINE_INS bool ast_bracketlist_foreach_member(
    const struct ast_node *n,
    exprlist_cb cb,
    void *user
);
