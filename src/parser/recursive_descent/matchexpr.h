#ifndef LFR_PARSER_MATCH_EXPRESSION_H
#define LFR_PARSER_MATCH_EXPRESSION_H

#include <stdbool.h>
#include <lexer/tokens.h>

struct parser;

#define TOKEN_IS_POSSIBLE_MATCH_EXPRESSION(tok_) (tok_ && (tok_)->type == TOKEN_KW_MATCH)

/**
 * conditional_branch = expression TOKEN_SM_OCBRACE block TOKEN_SM_CCBRACE
 */

/**
 * match_pattern = type_description
 * match_case = match_pattern TOKEN_SM_THICKARROW expression
 *            / NIL
 * match_cases = match_case match_cases
 * match_expression = TOKEN_KW_MATCH identifier TOKEN_SM_OCBRACE match_cases TOKEN_SM_CCBRACE
 *
 * @param p               The parser instance 
 * @param have_header     True if it's an expression with a match(..) { } header
 *                        and false if it's a body of a function
 * @param expect_it       If true, failure to parse is an error.
 * @return                The parsed match expression node.
 */
struct ast_node *parser_acc_matchexpr(struct parser *p,
                                      bool have_header,
                                      bool expect_it);

#endif
