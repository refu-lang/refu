#include <ir/parser/rirparser.h>
#include <front_ctx.h>
#include <utils/common_strings.h>
#include <parser/parser_common.h>
#include <Utils/memory.h>
#include <String/rf_str_corex.h>

#include <inpfile.h>
#include <ir/rir.h>

i_INLINE_INS void rir_pctx_init(struct rir_pctx *ctx, struct rir *r);
i_INLINE_INS void rir_pctx_set_id(struct rir_pctx *ctx, const struct RFstring *id);
i_INLINE_INS void rir_pctx_reset_id(struct rir_pctx *ctx);

i_INLINE_INS struct rir_parser *parser_common_to_rirparser(const struct parser_common* c);
struct rir_parser *rir_parser_create(struct front_ctx *front, struct inpfile *f, struct lexer *lex, struct info_ctx *info)
{
    struct rir_parser *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_parser_init(ret, front, f, lex, info)) {
        free(ret);
        return NULL;
    }
    return ret;
}

bool rir_parser_init(
    struct rir_parser *p,
    struct front_ctx *front,
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *info
)
{
    RF_STRUCT_ZERO(p);
    parser_common_init(&p->cmn, PARSER_RIR, front, f, lex, info);
    // initialize the parsing buffer
    if (!rf_stringx_init_buff(&p->buff, 1024, "")) {
        return false;
    }

    return true;
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

static struct rir_object *parse_outer_assignment(struct rir_parser *p, struct token *tok, const struct RFstring *name)
{
    switch (rir_toktype(tok)) {
    case RIR_TOK_UNIONDEF:
        return rir_parse_typedef(p, name, true);
    case RIR_TOK_TYPEDEF:
        return rir_parse_typedef(p, name, false);
    case RIR_TOK_GLOBAL:
        return rir_parse_global(p, name);
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

struct rir_object *rir_accept_identifier_var(
    struct rir_parser *p,
    struct token *tok,
    struct rir_object *(*assignment_parser)(struct rir_parser*, struct token*, const struct RFstring *name)
)
{
    struct token *tok2;
    struct token *tok3;
    tok2 = lexer_lookahead(parser_lexer(p), 2);
    tok3 = lexer_lookahead(parser_lexer(p), 3);
    if ((!tok2 || !tok3) || rir_toktype(tok2) != RIR_TOK_OP_ASSIGN) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Expected an assignment after \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        return NULL;
    }

    // consume identifier variable ($N)
    lexer_curr_token_advance(parser_lexer(p));
    // consume '='
    lexer_curr_token_advance(parser_lexer(p));

    return assignment_parser(p, tok3, ast_identifier_str(tok->value.value.ast));
}

static bool rir_parse_outer_statement(struct rir_parser *p)
{
    struct token *tok;
    if (!(tok = lexer_lookahead(parser_lexer(p), 1))) {
        return false;
    }

    switch(rir_toktype(tok)) {
    case RIR_TOK_IDENTIFIER_VARIABLE:
        return rir_accept_identifier_var(p, tok, parse_outer_assignment);
    case RIR_TOK_FNDECL:
        return rir_parse_fndecl(p);
    case RIR_TOK_FNDEF:
        return rir_parse_fndef(p);
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
    struct rir *r = rir_create();
    rir_pctx_init(&p->ctx, r);
    while (rir_parse_outer_statement(p)) {
        ;
    }

    // if we got any error messages we failed
    if (info_ctx_has(p->cmn.info, MESSAGE_ANY)) {
        rir_destroy(r);
        return false;
    }

    if (!p->ctx.module_created) {
        // no module name found, no module created, so this as the main module
        if (!rf_string_copy_in(&r->name, &g_str_main)) {
            return false;
        }
        return front_ctx_make_main(parser_front(p), NULL, r);
    }

    return true;
}

i_INLINE_INS struct rir *rir_parser_rir(const struct rir_parser *p);
