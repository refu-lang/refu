#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

#include <Utils/sanity.h>

static bool rir_parse_fn_common(
    struct rir_parser *p,
    bool foreign,
    struct rir_fndecl *decl
)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    // consume fndef/fndecl
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_FNDEF || rir_toktype(tok) == RIR_TOK_FNDECL,
              "Expected either fndef or fndecl rir token");
    lexer_curr_token_advance(parser_lexer(p));
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected '(' after '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        return false;
    }

    // get function name
    if (!(tok = lexer_expect_token(parser_lexer(p), RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier as first argument of '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        return false;
    }
    const struct RFstring *fnname = ast_identifier_str(tok->value.value.ast);

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SEMICOLON)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ';' after the function name."
        );
        return false;
    }

    // get function's arguments
    struct rir_type_arr args;
    if (!rir_parse_typearr(p, &args, RIR_TOK_SEMICOLON)) {
        return false;
    }
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SEMICOLON)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ';' after the arguments of a function."
        );
        goto fail_free_args;
    }
    // if there is only a nil, then this means we got no args so provide empty args to further init functions
    if (darray_size(args) == 1 && rir_type_is_specific_elementary(darray_item(args, 0), ELEMENTARY_TYPE_NIL)) {
        darray_clear(args);
    }

    // get function's return type
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("function return type");
    struct rir_type *ret_type;
    if (!(ret_type = rir_parse_type(p, &lmsg))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected the function's return type as third argument."
        );
        goto fail_free_args;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ')' at the end of the '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        goto fail_free_ret_type;
    }

    if (!rir_fndecl_init(
            decl,
            fnname,
            &args,
            ret_type,
            foreign,
            RIRPOS_PARSE,
            &p->ctx
        )) {
        goto fail_free_ret_type;
    }
    return true;

fail_free_ret_type:
    rir_type_destroy(ret_type, rir_parser_rir(p));
fail_free_args:
    darray_free(args);
    return false;
}

bool rir_parse_fndecl(struct rir_parser *p)
{
    struct rir_fndecl *decl;
    RF_MALLOC(decl, sizeof(*decl), return NULL);
    if (!rir_parse_fn_common(p, true, decl)) {
        free(decl);
        return false;
    }
    rf_ilist_add_tail(&rir_parser_rir(p)->functions, &decl->ln);
    return true;
}

bool rir_parse_fndef(struct rir_parser *p)
{
    // TODO: abstract the behaviour here so that it can be nicely streamlined
    //       between here and the testing code. Let's avoid code duplication
    struct rir_fndef *def;
    RF_MALLOC(def, sizeof(*def), return false);
    RF_STRUCT_ZERO(def);
    struct rir *r = rir_parser_rir(p);
    if (!rir_parse_fn_common(p, false, &def->decl)) {
        free(def);
        return false;
    }
    if (!rir_fndef_init_no_decl(def, RIRPOS_PARSE, &p->ctx)) {
        rir_fndef_destroy(def);
        return false;
    }

    // set this as the current function and add it to the list
    rir_data_curr_fn(&p->ctx) = def;
    rf_ilist_add_tail(&r->functions, &def->decl.ln);

    if (!rir_parse_bigblock(p, "function definition header")) {
        rf_ilist_delete_from(&r->functions, &def->decl.ln);
        rir_fndef_destroy(def);
        return false;
    }

    return true;

}
