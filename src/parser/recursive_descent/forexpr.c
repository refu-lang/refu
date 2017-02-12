#include "forexpr.h"

#include <ast/ast.h>
#include <ast/forexpr.h>
#include <ast/iterable.h>
#include <ast/constants.h>
#include <info/info.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include "common.h"
#include "identifier.h"
#include "block.h"

static struct ast_node *ast_parser_acc_range_member(struct ast_parser *p, struct token *tok)
{
    struct ast_node *n = NULL;
    if (!tok) {
        return NULL;
    }
    if (tok->type == TOKEN_IDENTIFIER) {
        n = ast_parser_acc_identifier(p);
    } else if (token_is_numeric_constant(tok)) {
        n = lexer_token_get_value(parser_lexer(p), tok);
    } else {
        return NULL;
    }
    // consume the identifier or constant
    lexer_curr_token_advance(parser_lexer(p));
    return n;
}


struct ast_node *ast_parser_acc_iterable(struct ast_parser *p)
{
    lexer_push(parser_lexer(p));
    struct ast_node *iterable;
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    struct token *tok2 = lexer_lookahead(parser_lexer(p), 2);
    if (!tok) {
        goto err;
    }

    if (tok2 && tok2->type == TOKEN_SM_COLON) {
        struct ast_node *start_node;
        struct ast_node *step_node = NULL;
        struct ast_node *end_node;
        if (!(start_node = ast_parser_acc_range_member(p, tok))) {
            goto err;
        }
        // consume ':'
        lexer_curr_token_advance(parser_lexer(p));

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!(end_node = ast_parser_acc_range_member(p, tok))) {
            parser_synerr(
                p,
                lexer_last_token_start(parser_lexer(p)),
                NULL,
                "An identifier or constant should follow the ':'"
            );
            goto err;
        }

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (tok && tok->type == TOKEN_SM_COLON) {
            step_node = end_node;
            //consume second ':'
            lexer_curr_token_advance(parser_lexer(p));

            tok = lexer_lookahead(parser_lexer(p), 1);
            if (!(end_node = ast_parser_acc_range_member(p, tok))) {
                parser_synerr(
                    p,
                    lexer_last_token_start(parser_lexer(p)),
                    NULL,
                    "An identifier or constant should follow the second ':'"
                );
                goto err;
            }
        }
        iterable = ast_iterable_create_range(
            start_node,
            step_node,
            end_node
        );

    } else { // simple iteration of a collection identified by identifier
        struct ast_node *n = ast_parser_acc_identifier(p);
        if (!n) {
            goto err;
        }
        iterable = ast_iterable_create_identifier(n);
    }

    if (!iterable) {
        RF_ERROR("Could not initialize an iterable ast_node");
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
        goto err_free_iterable;
    }

    if (!(n = ast_forexpr_create(start, ast_node_endmark(body), loopvar, iterable, body))) {
        RF_ERRNOMEM();
        goto err_free_body;
    }

    lexer_pop(parser_lexer(p));
    return n;

err_free_body:
    ast_node_destroy(body);
err_free_iterable:
    ast_node_destroy(iterable);
err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
