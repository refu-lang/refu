#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

struct rir_type *rir_parse_type(struct rir_parser *p, const struct RFstring *msg)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER, "Expected identifier");
    struct rir_type *ret = rir_type_byname(rir_parser_rir(p), ast_identifier_str(tok->value.value.ast));
    if (!ret) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Expected a type "RF_STR_PF_FMT" but provided string is not a recognized type",
            RF_STR_PF_ARG(msg)
        );
        return false;
    }
    // consume type identifier
    tok = lexer_next_token(parser_lexer(p));
    if (tok && rir_toktype(tok) == RIR_TOK_OP_MULTI) {
        ret = rir_type_set_pointer(&ret, true);
        // consume '*'
        lexer_next_token(parser_lexer(p));
    }

    return ret;
}

bool rir_parse_typearr(struct rir_parser *p, struct rir_type_arr *arr, enum rir_token_type ending_token)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok) {
        return false;
    }
    darray_init(*arr);

    static const struct RFstring locmsg = RF_STRING_STATIC_INIT("at type array");
    while (rir_toktype(tok) == RIR_TOK_IDENTIFIER) {
        struct rir_type *t = rir_parse_type(p, &locmsg);
        if (!t) {
            goto fail_free_arr;
        }

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok || (rir_toktype(tok) != ending_token &&
                     rir_toktype(tok) != RIR_TOK_SM_COMMA)) {
            rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                             "Expected either ',' or ')' after type identifier");
            goto fail_free_arr;
        }
        // add it to the type array
        darray_append(*arr, t);

        // check if we reached end of array
        if (rir_toktype(tok) == ending_token) {
            // succesfully exit the loop
            break;
        }
        // else consume the token and go to the next one
        tok = lexer_next_token(parser_lexer(p));
    }
    return true;

fail_free_arr:
    darray_free(*arr);
    return false;
}

struct rir_object *rir_parse_typedef(
    struct rir_parser *p,
    const struct RFstring *name,
    bool uniondef
)
{
#define i_DEFSTR "'%s'.", uniondef ? "uniondef" : "typedef"

    // consume 'typedef' / 'uniondef'
    lexer_curr_token_advance(parser_lexer(p));

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                      "Expected '(' after "i_DEFSTR);
        return NULL;
    }

    // parse the type array
    struct rir_type_arr arr;
    if (!rir_parse_typearr(p, &arr, RIR_TOK_SM_CPAREN)) {
        return NULL;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ')' at the end of the type definition"
        );
        goto fail_destroy_arr;
    }

    // finally create the typedef here
    struct rir_object *def = rir_typedef_create_obj(
        rir_parser_rir(p),
        p->ctx.common.current_fn,
        name,
        uniondef,
        &arr
    );
    if (!def) {
        goto fail_destroy_arr;
    }

    // success
#undef i_typestr
    return def;

fail_destroy_arr:
    rir_typearr_deinit(&arr, rir_parser_rir(p));
    return NULL;
}
