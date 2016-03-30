#ifndef LFR_PARSER_EXPRESSION_H
#define LFR_PARSER_EXPRESSION_H

#include <stdbool.h>

struct ast_node;
struct ast_parser;

#define EXPR_ELEMENT_START "an expression"

/**
 * expression = expr_level1 expression'
 *
 * expression' = TOKEN_OP_ASSIGN expr_level1 expression'
 *             / EMPTY
 *
 * expr_level1 = expr_level2 expr_level1'
 *
 * expr_level1' = TOKEN_OP_PLUS   expr_level2 expr_level1'
 *              / TOKEN_OP_MINUS  expr_level2 expr_level1'
 *              / EMPTY
 *
 * expr_level2 = expr_level3 expr_level2'
 *
 * expr_level2' = TOKEN_OP_MULTI expr_level3 expr_level2'
 *              / TOKEN_OP_DIV   expr_level3 expr_level2'
 *              / EMPTY
 *
 * expr_prefix_unary_ops = TOKEN_OP_AMPERSAND
 *                       / TOKEN_OP_INC
 *                       / TOKEN_OP_DEC
 *
 * expr_postfix_unary_ops = TOKEN_OP_INC
 *                        / TOKEN_OP_DEC
 *
 * expr_level3 = expr_factor
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
struct ast_node *ast_parser_acc_expression(struct ast_parser *p);

#endif
