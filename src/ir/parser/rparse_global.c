#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

struct rir_object *rir_parse_global(struct rir_parser *p, const struct RFstring *name)
{
    struct token *tok = NULL;
    // consume 'global'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_GLOBAL))) {
        return NULL;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a type identifier as first argument of 'global'.");
        return NULL;
    }
    struct ast_node *type_id = tok->value.value.ast;

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after the first argument of 'global'.");
        return NULL;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_STRING_LITERAL))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a string literal as second argument of 'global'.");
        return NULL;
    }
    struct ast_node *string_lit = tok->value.value.ast;

    // create and add it to the global literals
    struct rir_object *ret = rir_global_create_parsed(p, name, type_id, string_lit);
    if (!ret) {
        return NULL;
    }

    // consume the last parentheses
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a closing ')' at the end of 'global'.");
        return NULL;
    }

    return ret;
}
