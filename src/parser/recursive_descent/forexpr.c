#include "forexpr.h"

#include <ast/ast.h>
#include <ast/forexpr.h>
#include <info/info.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include "common.h"
#include "identifier.h"
#include "block.h"

struct ast_node *ast_parser_acc_forexpr(struct ast_parser *p)
{
    struct token *tok;
    const struct inplocation_mark *start;
    struct ast_node *loopvar;
    struct ast_node *iterable;
    struct ast_node *body;
    struct ast_node *n;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_KW_FOR) {
        return NULL;
    }
    lexer_push(parser_lexer(p));
    start = token_get_start(tok);
    //console 'for'
    lexer_curr_token_advance(parser_lexer(p));

    if (!(loopvar = ast_parser_acc_identifier(p))) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier for the loop variable after 'for'"
        );
        goto err;
    }

    if (!lexer_expect_token(parser_lexer(p), TOKEN_KW_IN)) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected 'in' after the for loop variable"
        );
        goto err;
    }

    if (!(iterable = ast_parser_acc_identifier(p))) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier for the iterable after 'in'"
        );
        goto err;
    }

    if (!(body = ast_parser_acc_block(p, true))) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a block following the 'for' expression"
        );
        goto err;
    }

    if (!(n = ast_forexpr_create(start, ast_node_endmark(body), loopvar, iterable, body))) {
        RF_ERRNOMEM();
        goto err_free_body;
    }

    return n;

err_free_body:
    ast_node_destroy(body);
err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
