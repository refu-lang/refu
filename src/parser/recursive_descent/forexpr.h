#ifndef LFR_PARSER_FOR_EXPRESSION_H
#define LFR_PARSER_FOR_EXPRESSION_H

#include <stdbool.h>
#include <lexer/tokens.h>

struct ast_parser;

#define TOKEN_IS_POSSIBLE_IFEXPR(tok_) (tok_ && (tok_)->type == TOKEN_KW_FOR)

/**
 * for_expresion = TOKEN_KW_FOR identifier TOKEN_KW_IN identifier TOKEN_SM_OCBRACE block TOKEN_SM_CCBRACE
 */
struct ast_node *ast_parser_acc_forexpr(struct ast_parser *p);

#endif
