#include "forexpr.h"

#include <ast/ast.h>
#include <ast/forexpr.h>
#include <ast/iterable.h>
#include <info/info.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include "common.h"
#include "identifier.h"
#include "block.h"

struct ast_node *ast_parser_acc_iterable(struct ast_parser *p)
{
    lexer_push(parser_lexer(p));
    struct ast_node *iterable;
    struct ast_node *n = ast_parser_acc_identifier(p);
    if (n) {
        iterable = ast_iterable_create_identifier(n);
    } else {

        struct token *tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok || !token_is_numeric_constant(tok)) {
            goto err;
        }
        const struct inplocation_mark *start = token_get_start(tok);

        struct ast_node *n = lexer_token_get_value(parser_lexer(p), tok);
        int64_t range_start;
        int64_t range_end;
        int64_t range_step;
        RF_ASSERT(ast_constant_get_integer(&n->constant, &range_start));
        lexer_curr_token_advance(parser_lexer(p));

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok || tok->type != TOKEN_SM_COLON) {
            parser_synerr(
                p,
                lexer_last_token_start(parser_lexer(p)),
                NULL,
                "Expected a ':' after the first number of a numeric range iterator"
            );
            goto err;
        }
        //consume ':'
        lexer_curr_token_advance(parser_lexer(p));

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok || !token_is_numeric_constant(tok)) {
            parser_synerr(
                p,
                lexer_last_token_start(parser_lexer(p)),
                NULL,
                "A range step or a range end integer should follow the ':'"
            );
            goto err;
        }
        const struct inplocation_mark *end = token_get_end(tok);
        RF_ASSERT(ast_constant_get_integer(&n->constant, &range_end));
        range_step = 1;

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (tok && tok->type == TOKEN_SM_COLON) {
            //consume second ':'
            lexer_curr_token_advance(parser_lexer(p));

            tok = lexer_lookahead(parser_lexer(p), 1);
            if (!tok || !token_is_numeric_constant(tok)) {
                parser_synerr(
                    p,
                    lexer_last_token_start(parser_lexer(p)),
                    NULL,
                    "A range end integer should follow the second ':'"
                );
                goto err;
            }
            end = token_get_end(tok);
            range_step = range_end;
            RF_ASSERT(ast_constant_get_integer(&n->constant, &range_end));
        }

        iterable = ast_iterable_create_range(
            start,
            end,
            range_start,
            range_step,
            range_end
        );
    }

    if (!iterable) {
        RF_ERROR("Could not initialize an iterale ast_node");
        goto err;
    }

    lexer_pop(parser_lexer(p));
    return iterable;

err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

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
    //consume 'for'
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

    if (!(iterable = ast_parser_acc_iterable(p))) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an iterable after 'in'"
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

    lexer_pop(parser_lexer(p));
    return n;

err_free_body:
    ast_node_destroy(body);
err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
