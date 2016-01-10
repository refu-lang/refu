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
    struct token *tok = lexer_lookahead(&p->lexer, 1);
    // consume fndef/fndecl
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_FNDEF || rir_toktype(tok) == RIR_TOK_FNDECL,
              "Expected either fndef or fndecl rir token");
    lexer_curr_token_advance(&p->lexer);
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected '(' after '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        return false;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected an identifier as first argument of '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        return false;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SEMICOLON)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected a ';' after the function name."
        );
        return NULL;
    }

    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("function return type");
    struct rir_type *ret_type = rir_parse_type(p, &lmsg); // can be NULL

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SEMICOLON)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected a ';' after the type."
        );
        goto fail_free_ret_type;
    }

    struct rir_type_arr args;
    if (!rir_parse_typearr(p, &args)) {
        goto fail_free_args;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected a ')' at the end of the '%s'.",
            foreign ? "fndecl" : "fndef"
        );
        goto fail_free_args;
    }

    if (!rir_fndecl_init(
            decl,
            ast_identifier_str(tok->value.value.ast),
            &args,
            ret_type,
            foreign,
            RIRPOS_PARSE,
            &p->ctx
        )) {
        goto fail_free_args;
    }
    return true;

fail_free_args:
    darray_free(args);
fail_free_ret_type:
    if (ret_type) {
        rir_type_destroy(ret_type, rir_parser_rir(p));
    }
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
    struct rir_fndef *def = rir_fndef_create_nodecl(RIRPOS_PARSE, &p->ctx);
    struct rir *r = rir_parser_rir(p);
    if (!rir_parse_fn_common(p, false, &def->decl)) {
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
