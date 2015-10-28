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

i_INLINE_DECL const struct type *ast_matchcase_matched_type(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    return n->matchcase.matched_type;
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

i_INLINE_DECL int ast_matchcase_index_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_CASE);
    RF_ASSERT(n->matchcase.match_idx != -1, "The index of the match case should have been set by now");
    return n->matchcase.match_idx;
}

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id);


i_INLINE_DECL size_t ast_matchexpr_cases_num(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    return n->matchexpr.match_cases_num;
}

/**
 * Check if a match expression has no cases, and should only appear inside another one
 */
i_INLINE_DECL bool ast_matchexpr_is_bodyless(const struct ast_node *n)
{
    return ast_matchexpr_cases_num(n) == 0;
}

/**
 * Check if a match expression has a match() header
 * @return true if yes and false if it's a function's body
 */
i_INLINE_DECL bool ast_matchexpr_has_header(const struct ast_node *n)
{
    return n->matchexpr.identifier_or_fnargtype->type == AST_IDENTIFIER;
}

/**
 * Will return the identifier that is inside match() if
 * this is not a headless match expression
 */
i_INLINE_DECL struct ast_node *ast_matchexpr_identifier(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    return ast_matchexpr_has_header(n) ? n->matchexpr.identifier_or_fnargtype : NULL;
}

/**
 * Only for a headless match expression Will return the function's arguments
 * type declaration whose body the match expression consists
 */
i_INLINE_DECL struct ast_node *ast_matchexpr_headless_args(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    RF_ASSERT(!ast_matchexpr_has_header(n), "Called with non-headless match expression");
    return n->matchexpr.identifier_or_fnargtype;
}

i_INLINE_DECL void ast_matchexpr_set_fnargs(struct ast_node *n,
                                            struct ast_node *fn_args)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    n->matchexpr.identifier_or_fnargtype = fn_args;
}

/**
 * Only for headless match expressions, get the symbol table record for the type.
 *
 * @param n            The headless match expression
 * @param st           The symbol table at which to search
 * @return             The symbol table record for the headless match expression's
 *                     type description or NULL if it's not found
 */
struct symbol_table_record* ast_matchexpr_headless_strec(const struct ast_node *n,
                                                         const struct symbol_table *st);

/**
 * Get the type that is being matched for @a n and save it as part of the expression.
 *
 * If this is a headless match expression then this is the type of the function
 * arguments, else it's the type of the identifier.
 *
 * @param n        The matchexpression whose matching type to compute and store
 * @param st       The symbol table to search for types while computing
 * @return         Returns the stored match expression in case of success.
 */
const struct type *ast_matchexpr_matched_type_compute(struct ast_node *n,
                                                      const struct symbol_table *st);
/**
 * Get the string representation of the type that is being matched for @a n
 *
 * @warning Need to wrap this in RFS_PUSH() and RFS_POP()
 */
const struct RFstring *ast_matchexpr_matched_type_str(const struct ast_node *n);

/**
 * Returns the matched type of a match expression.
 * To be called only after @ref ast_matchexpr_matched_type_compute()
 * @param n        The match expression whose matched type to return
 * @return         The matched type
 */
i_INLINE_DECL const struct type *ast_matchexpr_matched_type(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    RF_ASSERT(n->matchexpr.matching_type, "Match expression's matching type not yet computed");
    return n->matchexpr.matching_type;
}

/**
 * Get the string representation of the value of the type
 * that is being matched for @a n
 * 
 * If the matched type is an identifier then the value is the identifier's string
 * and if it's an anonymous type then the value is the output of 
 * type_get_unique_type_str() on the type.
 *
 * @warning Need to wrap this in RFS_PUSH() and RFS_POP()
 */
const struct RFstring *ast_matchexpr_matched_value_str(const struct ast_node *n);

/**
 * Determines the matched index of the type for each case. To be called
 * only during the rir forming stage.
 *
 * @param n          The match expression whose cases to have the indices set
 * @return           True for success and false for failure
 */
bool ast_matchexpr_cases_indices_set(struct ast_node *n);

void ast_matchexpr_add_case(struct ast_node *n, struct ast_node *mcase);

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
 * @return True if the what @ref ast_matchexpr_next_case() would return would be
 *         the last case and false otherwise
 */
bool ast_match_expr_next_case_is_last(const struct ast_node *matchexpr,
                                      struct ast_matchexpr_it *it);

/**
 * @return The match case at index @a i or NULL if it does not exist
 */
struct ast_node *ast_matchexpr_get_case(const struct ast_node *n, unsigned int i);

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
