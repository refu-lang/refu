#ifndef LFR_PARSER_EXPRESSION_H
#define LFR_PARSER_EXPRESSION_H

#include <stdbool.h>

struct ast_node;
struct parser;

#define EXPR_ELEMENT_START "a string literal, a numeric constant or an identifier"

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
 * expr_prefix_unary_ops = TOKEN_OP_AMPERSAND
 *                       / TOKEN_OP_INC
 *                       / TOKEN_OP_DEC
 *
 * expr_postfix_unary_ops = TOKEN_OP_INC
 *                        / TOKEN_OP_DEC
 *
 * expr_factor = expr_prefix_unary_ops expr_element
 *             / expr_element expr_postfix_unary_ops
 *             / expr_element
 *
 * expr_element = string_literal
 *              / numeric_constant
 *              / identifier
 *              / function_call
 *              / array_reference
 *              / TOKEN_SM_OPAREN expression TOKEN_SM_CPAREN
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
