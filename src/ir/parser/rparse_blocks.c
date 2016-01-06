#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir_convert.h>
#include <ir/rir.h>

#include <Utils/sanity.h>

struct rir_object *rir_parse_convert(struct rir_parser *p, struct rir *r)
{
    // consume convert
    lexer_curr_token_advance(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '(' after 'convert'.");
        return NULL;
    }

    struct rir_value *val = rir_parse_value(p, r, "as first argument of convert");
    if (!val) {
        return NULL;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after first argument of 'convert'.");
        return NULL;
    }

    struct rir_type *type = rir_parse_type(p, r, "second argument of 'convert'");
    if (!type) {
        return NULL;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' at the end of 'convert'.");
        return NULL;
    }

    struct rir_object *cnv = rir_convert_create_obj(val, type, RIRPOS_PARSE, &p->ctx);
    if (!cnv) {
        rir_value_destroy(val);
        rir_type_destroy(type, r);
    }
    return cnv;
}

static struct rir_object *parse_assignment(struct rir_parser *p, struct token *tok, const struct RFstring *name, struct rir *r)
{
    switch (rir_toktype(tok)) {
    case RIR_TOK_CONVERT:
        return rir_parse_convert(p, r);;
    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Unexpected rir token \""RF_STR_PF_FMT"\" after outer assignment to identifier.",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        break;
    }
    return NULL;
}

static struct rir_expression *rir_parse_expression(struct rir_parser *p, struct rir *r)
{
    struct token *tok;
    if (!(tok = lexer_lookahead(&p->lexer, 1))) {
        return NULL;
    }

    switch(rir_toktype(tok)) {
    case RIR_TOK_IDENTIFIER_VARIABLE:
        return rir_object_to_expr(rir_accept_identifier_var(p, tok, parse_assignment, r));
    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Unexpected rir token \""RF_STR_PF_FMT"\" during parsing",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        break;
    }
    return false;
}

static bool rir_parse_block(struct rir_parser *p, struct token *tok, struct rir *r)
{
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL,
              "Expected a label identifier at the beginning.");
    struct rir_block *b = rir_block_create(
        ast_identifier_str(tok->value.value.ast),
        RIRPOS_PARSE,
        &p->ctx
    );
    if (!b) {
        RF_ERROR("Could not create rir block while parsing");
        return false;
    }
    // consume the label identifier
    lexer_curr_token_advance(&p->lexer);

    struct rir_expression *expr;
    while ((expr = rir_parse_expression(p, r))) {
        // since b should be the current block add it there
        rir_common_block_add(&p->ctx.common, expr);
    }



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
