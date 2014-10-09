#ifndef LFR_PARSER_IF_EXPRESSION_H
#define LFR_PARSER_IF_EXPRESSION_H

struct parser;


/**
 * conditional_branch = expression TOKEN_SM_OCBRACE block TOKEN_SM_CCBRACE
 */

/**
 * fall_through_branch = TOKEN_KW_ELSE conditional_branch
 *
 * if_expression = TOKEN_KW_IF conditional_branch if_expression' [fall_through_branch]
 *
 * if_expression' = TOKEN_KW_ELIF conditional_branch if_expression'
 *                / EMPTY
 */
struct ast_node *parser_acc_ifexpr(struct parser *p);

#endif
