#ifndef LFR_AST_MATCHEXPR_H
#define LFR_AST_MATCHEXPR_H

#include <Definitions/inline.h>
#include <stdbool.h>
#include <ast/ast.h>

struct ast_node *ast_matchcase_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *pattern,
                                      struct ast_node *expression);

i_INLINE_DECL struct ast_node *ast_matchcase_pattern(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    return n->matchcase.pattern;
}

i_INLINE_DECL struct ast_node *ast_matchcase_expression(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    return n->matchcase.expression;
}

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id);



/**
 * Check if a match expression has no cases, and should only appear inside another one
 */
i_INLINE_DECL bool ast_matchexpr_is_bodyless(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    return ast_node_get_children_number(n) == 1;
}

i_INLINE_DECL struct ast_node *ast_matchexpr_identifier(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    return n->matchexpr.identifier;
}

typedef bool (*matchexpr_foreach_cb) (const struct ast_node *n, void *user_arg);
bool ast_matchexpr_foreach_case(const struct ast_node *n,
                                matchexpr_foreach_cb cb,
                                void *user_arg);

#endif
