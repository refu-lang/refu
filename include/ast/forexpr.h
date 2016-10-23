#ifndef LFR_AST_FOR_EXPRESSION_H
#define LFR_AST_FOR_EXPRESSION_H

#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>

#include <ast/ast.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_forexpr_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *loopvar,
    struct ast_node *iterable,
    struct ast_node *body
);

i_INLINE_DECL bool ast_forexpr_symbol_table_init(struct ast_node *n, struct module *m)
{
    AST_NODE_ASSERT_TYPE(n, AST_FOR_EXPRESSION);
    return symbol_table_init(&n->forexpr.st, m);
}

i_INLINE_DECL struct symbol_table* ast_forexpr_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FOR_EXPRESSION);
    return &n->forexpr.st;
}

i_INLINE_DECL struct ast_node* ast_forexpr_iterable_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FOR_EXPRESSION);
    return n->forexpr.iterable;
}

i_INLINE_DECL struct ast_node* ast_forexpr_loopvar_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FOR_EXPRESSION);
    return n->forexpr.loopvar;
}

i_INLINE_DECL struct ast_node* ast_forexpr_body_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FOR_EXPRESSION);
    return n->forexpr.body;
}

#endif
