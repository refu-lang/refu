#include "identifier.h"

#include <ast/ast.h>
#include <ast/identifier.h>

#include "arr.h"
#include "generics.h"

#include "common.h"

i_INLINE_INS struct ast_node *ast_parser_peek_identifer(struct ast_parser* p);
i_INLINE_INS struct ast_node *ast_parser_acc_identifier(struct ast_parser *p);

struct ast_node *ast_parser_acc_xidentifier(struct ast_parser *p, bool expect_it)
{
    struct ast_node *id;
    struct ast_node *xid;
    struct ast_node *genr;
    struct ast_node *arrspec;
    bool is_const = false;
    const struct inplocation_mark *start;
    const struct inplocation_mark *end;

    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok) {
        return NULL;
    }

    // parsing logic for the annotations to the identifier here
    if (tok->type == TOKEN_KW_CONST) {
        //consume 'const'
        lexer_curr_token_advance(parser_lexer(p));

        is_const = true;
        start = token_get_start(tok);
        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok) {
            parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
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
    lexer_curr_token_advance(parser_lexer(p));
    id = lexer_token_get_value(parser_lexer(p), tok);
    end = ast_node_endmark(id);

    tok = lexer_lookahead(parser_lexer(p), 1);
    genr = ast_parser_acc_genrattr(p, false);
    if (!genr && ast_parser_has_syntax_error(p)) {
        return NULL;
    }
    if (genr) {
        end = ast_node_endmark(genr);
    }

    arrspec = ast_parser_acc_arrspec(p);
    if (!arrspec && ast_parser_has_syntax_error(p)) {
        goto fail_free_genr;
    }
    if (arrspec) {
        end = ast_node_endmark(arrspec);
    }

    xid = ast_xidentifier_create(start, end, id, is_const, genr, arrspec);
    if (!xid) {
        RF_ERRNOMEM();
        goto fail_free_arrspec;
    }

    return xid;

fail_free_arrspec:
    if (arrspec) {
        ast_node_destroy(arrspec);
    }
fail_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
    return NULL;
}
