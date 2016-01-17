#ifndef LFR_PARSER_IF_EXPRESSION_H
#define LFR_PARSER_IF_EXPRESSION_H

#include <stdbool.h>
#include <lexer/tokens.h>

struct ast_parser;

#define TOKEN_IS_POSSIBLE_IFEXPR(tok_) (tok_ && (tok_)->type == TOKEN_KW_IF)

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
struct ast_node *ast_parser_acc_ifexpr(struct ast_parser *p, enum token_type if_type);

#endif
