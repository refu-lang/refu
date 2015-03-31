#ifndef LFR_AST_MATCHEXPR_H
#define LFR_AST_MATCHEXPR_H

#include <Definitions/inline.h>
#include <stdbool.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_matchcase_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *pattern,
                                      struct ast_node *expression);

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id);

#include <ast/ast.h>

/**
 * Check if a match expression has no cases, and should only appear inside another one
 */
i_INLINE_DECL bool ast_matchexpr_is_bodyless(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    return ast_node_get_children_number(n) == 1;
}
#endif
