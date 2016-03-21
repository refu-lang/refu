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
    return darray_size(n->arrspec.dimensions);
}


#endif
