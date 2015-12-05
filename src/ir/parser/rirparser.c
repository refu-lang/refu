#include <ir/parser/rirparser.h>
#include <Utils/memory.h>
#include <String/rf_str_corex.h>

#include <inpfile.h>
#include <ir/rir.h>

#include "rparse_global.h"

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

static bool rir_parse_single(struct rir_parser *p, struct rir *r)
{
    struct token *tok;
    tok = lexer_lookahead(&p->lexer, 1);

    switch((enum rir_token_type)tok->type) {
    case RIR_TOK_GLOBAL:
        return rir_parse_global(p, tok, r);
    default:
        RF_CRITICAL_FAIL("Unexpected rir token type during parsing");
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

    do {
        if (!rir_parse_single(p, r)) {
            goto fail;
        }
    } while (!inpfile_at_eof(p->file));

    // success
    return true;

fail:
    rir_destroy(r);
    return false;
}
