#ifndef LFR_PARSER_BLOCK_H
#define LFR_PARSER_BLOCK_H

#include <stdbool.h>

struct ast_parser;

#define TOKEN_IS_BLOCK_START(tok_) ((tok_) && (tok_)->type == TOKEN_SM_OCBRACE)

/**
 * block_element = expression
 *               | expression_statement
 *
 * block = TOKEN_SM_OCBRACE block_element TOKEN_SM_CCBRACE
 */
struct ast_node *ast_parser_acc_block(struct ast_parser *p, bool expect_braces);
#endif
