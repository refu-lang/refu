#include "block.h"

#include <ast/block.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include "common.h"
#include "expression.h"

struct ast_node *parser_acc_block(struct parser *p, bool expect_braces)
{
    struct ast_node *n;
    struct token *tok;
    struct ast_node *expr;
    struct inplocation_mark *start = NULL;
    struct inplocation_mark *end = NULL;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    if (expect_braces) {
        if (tok->type != TOKEN_SM_OCBRACE) {
            return NULL;
        }
        // consume '{'
        lexer_next_token(p->lexer);
        start = token_get_start(tok);
    }

    n = ast_block_create();
    if (!n) {
        RF_ERRNOMEM();
        return NULL;
    }

    //try to parse the first expression
    expr = parser_acc_expression(p);
    if (!expr) {
        if (!expect_braces) {
            goto err_free_block;
        }

        //else just find the terminating '}' and return
        tok = lexer_lookahead(p->lexer, 1);
        if (!tok || tok->type != TOKEN_SM_CCBRACE) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "Expected an expression or a '}' at block end");
            goto err_free_block;
        }
        //consume the '}'
        lexer_next_token(p->lexer);

        ast_node_set_start(n, start);
        ast_node_set_start(n, token_get_end(tok));
        return n; //empty block
    }

    if (!start) {
        start = ast_node_startmark(expr);
    }
    ast_node_add_child(n, expr);

    // now add expressions to the block
    expr = parser_acc_expression(p);
    while (expr) {
        ast_node_add_child(n, expr);
        end = ast_node_endmark(expr);
    }

    if (expect_braces) {
        tok = lexer_lookahead(p->lexer, 1);
        if (!tok || tok->type != TOKEN_SM_CCBRACE) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "Expected an expression or a '}' at block end");
            goto err_free_block;
        }
        //consume the '}'
        lexer_next_token(p->lexer);

        end = token_get_end(tok);
    }

    ast_node_set_start(n, start);
    ast_node_set_start(n, end);
    return n;

err_free_block:
        ast_node_destroy(n);
        return NULL;

}
