#include "identifier.h"

#include <ast/ast.h>
#include <ast/identifier.h>

#include "generics.h"

#include "common.h"

i_INLINE_INS struct ast_node *parser_acc_identifier(struct parser *p);
struct ast_node *parser_acc_xidentifier(struct parser *p, bool expect_it)
{
    struct token *tok;
    struct ast_node *id;
    struct ast_node *xid;
    struct ast_node *genr;
    bool is_const = false;
    const struct inplocation_mark *start;
    const struct inplocation_mark *end;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    // parsing logic for the annotations to the identifier here
    if (tok->type == TOKEN_KW_CONST) {
        //consume 'const'
        lexer_next_token(p->lexer);

        is_const = true;
        start = token_get_start(tok);
        tok = lexer_lookahead(p->lexer, 1);
        if (!tok) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "Expected an identifier after const");
            return NULL;
        }
    } else {
        start = token_get_start(tok);
    }

    if (tok->type != TOKEN_IDENTIFIER) {
        if (expect_it) {
            parser_synerr(p, token_get_end(tok), NULL,
                          "Expected an identifier");
        }
        return NULL;
    }
    //consume identifier
    lexer_next_token(p->lexer);
    id = token_get_value(tok);
    end = ast_node_endmark(id);

    tok = lexer_lookahead(p->lexer, 1);
    genr = parser_acc_genrattr(p, false); // can be NULL for example in: if a < 15 { .. }
    if (!genr && parser_has_syntax_error(p)) {
        return NULL;
    }
    if (genr) {
        end = ast_node_endmark(genr);
    }

    xid = ast_xidentifier_create(start, end, id, is_const, genr);
    if (!xid) {
        RF_ERRNOMEM();
        return NULL;
    }

    return xid;
}
