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

i_INLINE_DECL struct symbol_table *ast_matchcase_symbol_table_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    return n->matchcase.st;
}

i_INLINE_DECL void *ast_matchcase_symbol_table_set(struct ast_node *n, struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    return n->matchcase.st = st;
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

struct ast_matchexpr_it {
    const struct RFilist_head *lh;
    const struct RFilist_node *ln;
};

/**
 * Get the first case of a match expression
 * @param n    The match expression whose first case to get
 * @param it   An uninitalized iterator to hold the values of the iteration
 * @return     A pointer to the first match case node or NULL if there is none
 */
struct ast_node *ast_matchexpr_first_case(const struct ast_node *n,
                                          struct ast_matchexpr_it *it);
/**
 * Get the next case of a match expression
 * @param n    The match expression whose next case to get
 * @param it   An iterator that has been successfully used in either an 
 *             @ref ast_matchexpr_first_case() or an @ref ast_matchexpr_next_case()
 *             previously
 * @return     A pointer to the next match case node or NULL if all cases are exhausted
 */
struct ast_node *ast_matchexpr_next_case(const struct ast_node *n,
                                         struct ast_matchexpr_it *it);

/**
 * Iterate all matchcases
 * @param matchexpr_    The match expression whose cases to iterate
 * @param iterator_     A matchexpr iterator to utilize for the iteration
 * @param value_        An ast node pointer to hold the case value in each iteration
 */
#define ast_matchexpr_foreach(matchexpr_, iterator_, value_)            \
    for (value_ = ast_matchexpr_first_case((matchexpr_), (iterator_));  \
         value_;                                                        \
         value_ = ast_matchexpr_next_case((matchexpr_), (iterator_)))

#endif
