#ifndef LFR_PARSER_EXPRESSION_H
#define LFR_PARSER_EXPRESSION_H

#include <stdbool.h>

struct ast_node;
struct parser;

/**
 * expression = expr_term expression'
 *
 * expression' = TOKEN_OP_PLUS   expr_term expression'
 *             / TOKEN_OP_MINUS  expr_term expression'
 *             / EMPTY
 *
 * expr_term = expr_factor expr_term'
 *
 * expr_term' = TOKEN_OP_MULTI expr_factor expr_term'
 *            / TOKEN_OP_DIV   expr_factor expr_term'
 *            / EMPTY
 *
 * expr_factor = expr_element expr_factor'
 *
 * expr_factor' = TOKEN_OP_AMPERSAND expr_element expr_factor'
 *              / TOKEN_OP_INC expr_element expr_factor'
 *              / TOKEN_OP_DEC expr_element expr_factor'
 *              / EMPTY
 *
 *
 * expr_element = string_literal
 *              / numeric_constant
 *              / identifier
 *              / function_call
 *              / array_reference
 */
struct ast_node *parser_acc_expression(struct parser *p);

/**
 * expressions_list = expression expressions_list'
 *                  / EMPTY
 * expressions_list' = TOKEN_OP_COMMA expression expressions_list'
 *                   / EMPTY
 */
bool parser_acc_expressions_list(struct parser *p,
                                 struct ast_node *parent);

#endif
