#ifndef LFR_AST_VARDECL_H
#define LFR_AST_VARDECL_H

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *desc);

#include <ast/ast.h>

i_INLINE_DECL struct ast_node *ast_vardecl_desc_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);
    return n->vardecl.desc;
}
#endif
