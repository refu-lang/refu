#ifndef LFR_PARSER_VARIABLE_DECLARATION_H
#define LFR_PARSER_VARIABLE_DECLARATION_H

struct ast_node;
struct parser;

#define TOKENS_ARE_POSSIBLE_VARDECL(tok1_, tok2_)           \
    (tok1_ && tok2_ && (tok1_)->type == TOKEN_IDENTIFIER && \
     (tok2_)->type == TOKEN_SM_COLON)

/**
 * variable_declaration = type_leaf
 */
struct ast_node *parser_acc_vardecl(struct parser *p);

#endif
