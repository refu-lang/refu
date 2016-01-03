#include <ir/parser/rirparser_functions.h>

#include <ir/parser/rirparser.h>
#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

static bool rir_parse_block(struct rir_parser *p, struct token *tok, struct rir *r)
{
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL,
              "Expected a label identifier at the beginning.");
    // TODO
    return true;

}

static bool rir_parse_bigblock(struct rir_parser *p, struct rir *r, const char *position)
{
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '{' after the %s.", position);
        return false;
    }

    struct token *tok;
    while ((tok = lexer_lookahead(&p->lexer, 1)) &&
           rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL) {
        if (!rir_parse_block(p, tok, r)) {
            return false;
        }
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '}' at the end of the block.");
        return false;
    }
    return true;
}

bool rir_parse_fndef(struct rir_parser *p, struct rir *r)
{
    struct token *tok;
    // consume fndef
    lexer_curr_token_advance(&p->lexer);
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected '(' after 'fndef'.");
        return false;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected an identifier as first argument of 'fndef'.");
        return false;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SEMICOLON)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected a ';' after the function name.");
        return false;
    }

    struct rir_type *ret_type = rir_parse_type(p, r); // can be NULL

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SEMICOLON)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected a ';' after the type.");
        return false;
    }

    struct rir_fndef *def = rir_fndef_create_from_parsing(
        ast_identifier_str(tok->value.value.ast),
        ret_type,
        r,
        p
    );
    if (!def) {
        return false;
    }

    // finally set this as the current function and add it to the list
    p->curr_fn = def;
    rf_ilist_add_tail(&r->functions, &def->decl.ln);

    if (!rir_parse_bigblock(p, r, "function definition header")) {
        rf_ilist_delete_from(&r->functions, &def->decl.ln);
        rir_fndef_destroy(def);
        return false;
    }

    return true;

}
