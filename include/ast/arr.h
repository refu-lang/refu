#ifndef LFR_AST_ARR_H
#define LFR_AST_ARR_H

#include <ast/ast_utils.h>
#include <ast/ast.h>
#include <lexer/tokens.h>

struct inplocation_mark;

struct ast_node *ast_arrspec_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct arr_ast_nodes *dimensions
);

i_INLINE_DECL unsigned int ast_arrspec_dimensions_num(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_ARRAY_SPEC);
    return darray_size(n->children);
}

struct ast_node *ast_bracketlist_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *args
);

/**
 * Get or create the members of a bracket list.
 *
 * The first time this function is called the members array is created
 * and then all subsequent calls simply return it
 *
 * @param n    The ast bracketlist whose members to get/create
 * @return     the members array of the bracketlist
 */
struct arr_ast_nodes *ast_bracketlist_members(struct ast_node *n);


/**
 * A way to iterate a bracketlist's members before typechecking creates
 * the array of ast_node members
 *
 * @param n        The bracketlist whose ast nodes to iterate
 * @param cb       The callback function to call for each bracket list member
 * @param user     The extra user argument to provide to the callback
 */
i_INLINE_DECL bool ast_bracketlist_foreach_member(
    const struct ast_node *n,
    exprlist_cb cb,
    void *user)
{
    AST_NODE_ASSERT_TYPE(n, AST_BRACKET_LIST);
    RF_ASSERT(
        n->state < AST_NODE_STATE_TYPECHECK_1,
        "Do not use this function after typechecking"
    );
    struct ast_node *expr = darray_item(n->children, 0);
    if (!expr) {
        return true;
    }
    return ast_foreach_expr(expr, cb, user);
}

#endif
