#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

#include <Utils/sanity.h>

static bool rir_parse_block(struct rir_parser *p, struct token *tok, struct rir *r)
{
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL,
              "Expected a label identifier at the beginning.");
    // TODO
    return true;
}

bool rir_parse_bigblock(struct rir_parser *p, struct rir *r, const char *position)
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
