#ifndef LFR_PARSER_ARRAY_REFERENCE_H
#define LFR_PARSER_ARRAY_REFERENCE_H

struct parser;

#define TOKENS_ARE_POSSIBLE_ARRAYREF(tok1_, tok2_)                      \
    (tok1_ && tok2_ && (tok1_)->type == TOKEN_IDENTIFIER &&             \
     (tok2_)->type == TOKEN_SM_OSBRACE)

/**
 * array_reference = identifier TOKEN_SM_OSBRACE expression TOKEN_SM_CSBRACE
 */
struct ast_node *parser_acc_arrayref(struct parser *p);
#endif
