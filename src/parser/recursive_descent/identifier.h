#ifndef LFR_PARSER_IDENTIFIER_H
#define LFR_PARSER_IDENTIFIER_H
#include <Definitions/inline.h>
struct parser;

#include <parser/parser.h>
#include <lexer/lexer.h>

i_INLINE_DECL struct ast_node *parser_acc_identifier(struct parser *p)
{
    struct token *tok;
    tok = lexer_next_token(p->lexer);
    if (tok && tok->type != TOKEN_KW_TYPE) {
        return NULL;
    }
    return token_get_identifier(tok);
}
/**
 * annotated_identifier = ["const"] identifier [generic_attributes]
 */
struct ast_node *parser_acc_xidentifier(struct parser *p);
#endif
