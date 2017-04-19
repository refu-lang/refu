#ifndef LFR_PARSER_IDENTIFIER_H
#define LFR_PARSER_IDENTIFIER_H
#include <rfbase/defs/inline.h>

#include <parser/parser.h>
#include <lexer/lexer.h>

struct ast_parser;

#define XIDENTIFIER_START_STR "'const' or identifier"
#define XIDENTIFIER_START_COND(tok_)            \
    ((tok_) &&                                                          \
     ((tok_)->type == TOKEN_KW_CONST || (tok_)->type == TOKEN_IDENTIFIER))

// start of an annotated identifier that does have to start with some special
// attributes
#define XIDENTIFIER_START_SPECIAL_COND(tok_) \
    (tok_ && (tok_)->type == TOKEN_KW_CONST)

i_INLINE_DECL struct ast_node *ast_parser_peek_identifer(struct ast_parser* p)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_IDENTIFIER) {
        return NULL;
    }
    // return the identifier token but let the lexer retain ownership
    lexer_curr_token_advance(parser_lexer(p));
    return lexer_token_get_value_but_keep_ownership(parser_lexer(p), tok);
}

i_INLINE_DECL struct ast_node *ast_parser_acc_identifier(struct ast_parser *p)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_IDENTIFIER) {
        return NULL;
    }
    // consume the identifier token and return it
    lexer_curr_token_advance(parser_lexer(p));
    return lexer_token_get_value(parser_lexer(p), tok);
}

/**
 * annotated_identifier = ["const"] identifier [generic_attributes] [array_specifier]
 */
struct ast_node *ast_parser_acc_xidentifier(struct ast_parser *p, bool expect_it);
#endif
