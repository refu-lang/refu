#include <ir/parser/rirparser.h>

#include <ir/rir_convert.h>
#include <ir/rir_value.h>
#include <ir/rir_type.h>
#include <ir/rir_expression.h>

struct rir_object *rir_parse_convert(struct rir_parser *p)
{
    // consume 'convert'
    lexer_curr_token_advance(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '(' after 'convert'.");
        return NULL;
    }

    struct rir_value *val = rir_parse_value(p, "as first argument of convert");
    if (!val) {
        return NULL;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after first argument of 'convert'.");
        goto fail_destroy_val;
    }

    struct rir_type *type = rir_parse_type(p, "second argument of 'convert'");
    if (!type) {
        goto fail_destroy_val;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' at the end of 'convert'.");
        goto fail_destroy_type;
    }

    struct rir_object *cnv = rir_convert_create_obj(val, type, RIRPOS_PARSE, &p->ctx);
    if (!cnv) {
        goto fail_destroy_type;
    }
    return cnv;

fail_destroy_type:
    rir_type_destroy(type, rir_parser_rir(p));
fail_destroy_val:
    rir_value_destroy(val);
    return NULL;
}

struct rir_object *rir_parse_write(struct rir_parser *p)
{
    // consume 'write'
    struct token *start = lexer_curr_token_advance(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '(' after 'write'.");
        return NULL;
    }

    struct rir_type *type = rir_parse_type(p, "first argument of 'write'");
    if (!type) {
        return NULL;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after first argument of 'write'.");
        goto fail_destroy_type;
    }

    struct rir_value *dstval = rir_parse_value(p, "as second argument of 'write'");
    if (!dstval) {
        goto fail_destroy_type;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after second argument of 'write'.");
        goto fail_destroy_dst;
    }

    struct rir_value *srcval = rir_parse_value(p, "as second argument of 'write'");
    if (!srcval) {
        goto fail_destroy_dst;
    }

    struct token *end;
    if (!(end = lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' at the end of 'write'.");
        goto fail_destroy_src;
    }

    if (!rir_type_identical(type, dstval->type) || !rir_type_identical(dstval->type, srcval->type)) {
        rirparser_synerr(
            p,
            token_get_start(start),
            token_get_end(end),
            "Type mismatch at arguments of 'write'"
        );
        goto fail_destroy_src;
    }

    struct rir_object *wrt;
    if (!(wrt = rir_write_create_obj(dstval, srcval, RIRPOS_PARSE, &p->ctx))) {
        goto fail_destroy_src;
    }
    return wrt;

fail_destroy_src:
    rir_value_destroy(srcval);
fail_destroy_dst:
    rir_value_destroy(dstval);
fail_destroy_type:
    rir_type_destroy(type, rir_parser_rir(p));
    return NULL;
}
