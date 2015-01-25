#ifndef LFR_AST_VARDECL_H
#define LFR_AST_VARDECL_H

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *desc);

#include <ast/ast.h>
#include <ast/type.h>

i_INLINE_DECL struct ast_node *ast_vardecl_desc_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);
    return n->vardecl.desc;
}

i_INLINE_DECL const struct RFstring *ast_vardecl_get_name(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);
    struct ast_node *left = ast_typedesc_left(n->vardecl.desc);
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    return ast_identifier_str(left);
}
#endif
