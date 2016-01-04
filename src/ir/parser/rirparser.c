#include <ir/parser/rirparser.h>
#include <Utils/memory.h>
#include <String/rf_str_corex.h>

#include <inpfile.h>
#include <ir/rir.h>

i_INLINE_INS void rir_pctx_init(struct rir_pctx *ctx, struct rir *r);

struct rir_parser *rir_parser_create(const struct RFstring *name,
                                     const struct RFstring *contents)
{
    struct rir_parser *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_parser_init(ret, name, contents)) {
        free(ret);
        return NULL;
    }
    return ret;
}

bool rir_parser_init(struct rir_parser *p,
                     const struct RFstring *name,
                     const struct RFstring *contents)
{
    RF_STRUCT_ZERO(p);
    // initialize the parsing buffer
    if (!rf_stringx_init_buff(&p->buff, 1024, "")) {
        return false;
    }

    p->file = contents
        ? inpfile_create_from_string(name, contents)
        : inpfile_create(name);
    if (!p->file) {
        goto free_buffer;
    }

    p->info = info_ctx_create(p->file);
    if (!p->info) {
        goto free_file;
    }

    if (!lexer_init(&p->lexer, p->file, p->info, true)) {
        goto free_info;
    }

    return true;

free_info:
    info_ctx_destroy(p->info);
free_file:
    inpfile_destroy(p->file);
free_buffer:
    rf_stringx_deinit(&p->buff);
    return false;
}

void rir_parser_destroy(struct rir_parser *p)
{
    rir_parser_deinit(p);
    free(p);
}

void rir_parser_deinit(struct rir_parser *p)
{
    rf_stringx_deinit(&p->buff);
}

static bool rir_accept_identifier_var(struct rir_parser *p, struct token *tok, struct rir *r)
{
    struct token *tok2;
    struct token *tok3;
    tok2 = lexer_lookahead(&p->lexer, 2);
    tok3 = lexer_lookahead(&p->lexer, 3);
    if ((!tok2 || !tok3) || rir_toktype(tok2) != RIR_TOK_OP_ASSIGN) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Expected an assignment after \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        return false;
    }

    switch (rir_toktype(tok3)) {
    case RIR_TOK_UNIONDEF:
        return rir_parse_typedef(p, tok, true, r);
    case RIR_TOK_TYPEDEF:
        return rir_parse_typedef(p, tok, true, r);
    default:
        rirparser_synerr(
            p,
            token_get_start(tok3),
            NULL,
            "Unexpected rir token \""RF_STR_PF_FMT"\" after assignment to identifier.",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok3)))
        );
        break;
    }

    return false;
}

static bool rir_parse_single(struct rir_parser *p, struct rir *r)
{
    struct token *tok;
    if (!(tok = lexer_lookahead(&p->lexer, 1))) {
        return false;
    }

    switch(rir_toktype(tok)) {
    case RIR_TOK_GLOBAL:
        return  rir_parse_global(p, tok, r);
    case RIR_TOK_IDENTIFIER_VARIABLE:
        return rir_accept_identifier_var(p, tok, r);
    case RIR_TOK_FNDECL:
        return rir_parse_fndecl(p, r);
    case RIR_TOK_FNDEF:
        return rir_parse_fndef(p, r);
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

bool rir_parse(struct rir_parser *p)
{
    if (!lexer_scan(&p->lexer)) {
        return false;
    }
    struct rir *r = rir_create();
    rir_pctx_init(&p->ctx, r);
    while (rir_parse_single(p, r)) {
        ;
    }

    if (info_ctx_has(p->info, MESSAGE_ANY)) {
        rir_destroy(r);
        return false;
    }

    return true;
}
