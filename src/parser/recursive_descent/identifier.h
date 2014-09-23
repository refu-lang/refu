#ifndef LFR_PARSER_IDENTIFIER_H
#define LFR_PARSER_IDENTIFIER_H
#include <Definitions/inline.h>
struct parser;

#include <parser/parser.h>
#include <lexer/lexer.h>

#define XIDENTIFIER_START_STR "'const' or identifier"
#define XIDENTIFIER_START_COND(tok_)            \
    ((tok_) &&                                                          \
     ((tok_)->type == TOKEN_KW_CONST || (tok_)->type == TOKEN_IDENTIFIER))

// start of an annotated identifier that does have to start with some special
// attributes
#define XIDENTIFIER_START_SPECIAL_COND(tok_) \
    (tok_ && (tok_)->type == TOKEN_KW_CONST)

i_INLINE_DECL struct ast_node *parser_acc_identifier(struct parser *p)
{
    struct token *tok;
    tok = lexer_next_token(p->lexer);
    if (tok && tok->type != TOKEN_IDENTIFIER) {
        return NULL;
    }
    return token_get_identifier(tok);
}
/**
 * annotated_identifier = ["const"] identifier [generic_attributes]
 */
struct ast_node *parser_acc_xidentifier(struct parser *p);
#endif
