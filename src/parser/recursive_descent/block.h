#ifndef LFR_PARSER_BLOCK_H
#define LFR_PARSER_BLOCK_H

#include <stdbool.h>

struct parser;

/**
 * block = TOKEN_SM_OCBRACE expressions TOKEN_SM_CCBRACE
 */
struct ast_node *parser_acc_block(struct parser *p, bool expect_braces);
#endif
