#ifndef LFR_PARSER_VARIABLE_DECLARATION_H
#define LFR_PARSER_VARIABLE_DECLARATION_H

struct ast_node;
struct ast_parser;

#define TOKENS_ARE_POSSIBLE_VARDECL(tok1_, tok2_)           \
    (tok1_ && tok2_ && (tok1_)->type == TOKEN_IDENTIFIER && \
     (tok2_)->type == TOKEN_SM_COLON)

/**
 * variable_declaration = type_leaf [ TOKEN_OP_ASSIGN expression]
 */
struct ast_node *ast_parser_acc_vardecl(struct ast_parser *p);

#endif
