#include "identifier.h"

#include <ast/ast.h>
#include <ast/identifier.h>

#include "generics.h"

#include "common.h"

i_INLINE_INS struct ast_node *parser_acc_identifier(struct parser *p);
struct ast_node *parser_acc_xidentifier(struct parser *p)
{
    struct token *tok;
    struct ast_node *id;
    struct ast_node *xid;
    struct ast_node *genr;
    bool is_const = false;
    struct inplocation_mark *start;

    lexer_push(p->lexer);

    tok = lexer_next_token(p->lexer);
    if (!tok) {
        goto not_found;
    }
    // parsing logic for the annotations to the identifier here
    if (tok->type == TOKEN_KW_CONST) {
        is_const = true;
        start = token_get_start(tok);
        tok = lexer_next_token(p->lexer);
        if (!tok) {
            parser_synerr(p, lexer_last_token_location(p->lexer),
                          "Expected an identifier after const");
            goto not_found;
        }
    }

    if (tok->type != TOKEN_IDENTIFIER) {
        parser_synerr(p, token_get_loc(tok),
                      "Expected an identifier after const");
        goto not_found;
    }
    id = tok->value.identifier;

    genr = parser_acc_genrattr(p);
    if (!genr || parser_has_synerr(p)) {
        goto not_found;
    }

    xid = ast_xidentifier_create(start, ast_node_endmark(id),
                                 id, is_const, genr);
    if (!xid) {
        //TODO: error
        goto not_found;
    }

    lexer_pop(p->lexer);
    return xid;
not_found:
    lexer_rollback(p->lexer);
return NULL;
}
