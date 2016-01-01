#include "rparse_global.h"

#include <ir/parser/rirparser.h>
#include <lexer/lexer.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

bool rir_parse_global(struct rir_parser *p, struct token *tok, struct rir *r)
{
    // consume global
    lexer_next_token(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected '(' after 'global'.");
        return false;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected an identifier as first argument of 'global'.");
        return false;
    }
    struct ast_node *name_id = tok->value.value.ast;

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after the first argument of 'global'.");
        return false;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected an identifier as second argument of 'global'.");
        return false;
    }
    struct ast_node *type_id = tok->value.value.ast;

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after the second argument of 'global'.");
        return false;
    }

    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_STRING_LITERAL))) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a string literal as third argument of 'global'.");
        return false;
    }
    struct ast_node *string_lit = tok->value.value.ast;

    // create and add it to the global literals
    struct rir_object *ret = rir_global_create_parsed(p, r, name_id, type_id, string_lit);
    if (!ret) {
        return false;
    }
    if (!strmap_add(&r->global_literals, &rir_object_value(ret)->literal, ret)) {
        RF_ERROR("Failed to add a string literal to the gloal string maps during parsing");
        return false;
    }

    // consume the last parentheses
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a closing ')' at the end of 'global'.");
        return false;
    }

    return true;
}
