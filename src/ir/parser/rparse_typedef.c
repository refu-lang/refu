#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

struct rir_type *rir_parse_type(struct rir_parser *p, struct rir *r, const char *msg)
{
    struct token *tok = lexer_lookahead(&p->lexer, 1);
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER, "Expected identifier");
    struct rir_type *ret = rir_type_byname(r, ast_identifier_str(tok->value.value.ast));
    if (!ret) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Expected a type %s but provided string is not a recognized type",
            msg
        );
        return false;
    }
    // consume type identifier
    tok = lexer_next_token(&p->lexer);
    if (tok && rir_toktype(tok) == RIR_TOK_OP_MULTI) {
        ret->is_pointer = true;
        // consume '*'
        lexer_next_token(&p->lexer);
    }

    return ret;
}

bool rir_parse_typearr(struct rir_parser *p, struct rir_type_arr *arr, struct rir *r)
{
    struct token *tok = lexer_lookahead(&p->lexer, 1);
    if (!tok) {
        return false;
    }
    darray_init(*arr);

    while (rir_toktype(tok) == RIR_TOK_IDENTIFIER) {
        struct rir_type *t = rir_parse_type(p, r, "at type array");
        if (!t) {
            goto fail_free_arr;
        }

        tok = lexer_lookahead(&p->lexer, 1);
        if (!tok || (rir_toktype(tok) != RIR_TOK_SM_CPAREN &&
                     rir_toktype(tok) != RIR_TOK_SM_COMMA)) {
            rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                             "Expected either ',' or ')' after type identifier");
            goto fail_free_arr;
        }
        // add it to the type array
        darray_append(*arr, t);

        // check if we reached end of array
        if (rir_toktype(tok) == RIR_TOK_SM_CPAREN) {
            // succesfully exit the loop
            break;
        }
        // else consume the token and go to the next one
        tok = lexer_next_token(&p->lexer);
    }
    return true;

fail_free_arr:
    darray_free(*arr);
    return false;
}

struct rir_object *rir_parse_typedef(
    struct rir_parser *p,
    const struct RFstring *name,
    bool uniondef,
    struct rir *r
)
{
#define i_DEFSTR "'%s'.", uniondef ? "uniondef" : "typedef"

    // consume 'typedef' / 'uniondef'
    lexer_curr_token_advance(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected '(' after "i_DEFSTR);
        return NULL;
    }

    // parse the type array into the typedef
    struct rir_type_arr arr;
    if (!rir_parse_typearr(p, &arr, r)) {
        return NULL;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected a ')' at the end of the type definition"
        );
        darray_free(arr);
        return NULL;
    }

    // finally create the typedef here
    struct rir_object *def = rir_typedef_create_obj(
        r,
        p->ctx.common.current_fn,
        name,
        uniondef,
        &arr
    );
    if (!def) {
        rir_typearr_deinit(&arr, r);
    }

#undef i_typestr
    return def;
}
