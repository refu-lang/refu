#include "block.h"

#include <ast/block.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include <ast/returnstmt.h>
#include "common.h"
#include "expression.h"
#include "type.h"

#define TOKEN_IS_RETURNSTMT_START(tok_) ((tok_) && (tok_)->type == TOKEN_KW_RETURN)
static struct ast_node *parser_acc_return_statement(struct parser *p)
{
    struct token *tok;
    struct ast_node *expr;
    struct ast_node *n = NULL;
    const struct inplocation_mark *start;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    if (tok->type != TOKEN_KW_RETURN) {
        return NULL;
    }
    start = token_get_start(tok);
    //consume the return keyword
    lexer_curr_token_advance(p->lexer);


    expr = parser_acc_expression(p);
    if (!expr) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected an expression for the return statement");
        goto end;
    }

    n = ast_returnstmt_create(start, ast_node_endmark(expr), expr);
    if (!n) {
        RF_ERROR("Could not create a return statement during parsing");
    }

end:
    return n;
}

static struct ast_node *parser_acc_expr_statement(struct parser *p)
{
    struct token *tok;
    struct ast_node *n = NULL;

    tok = lexer_lookahead(p->lexer, 1);

    if (TOKEN_IS_BLOCK_START(tok)) {
        n = parser_acc_block(p, true);
    } else if (TOKEN_IS_TYPEDECL_START(tok)) {
        n = parser_acc_typedecl(p);
    } else if (TOKEN_IS_RETURNSTMT_START(tok)) {
        n = parser_acc_return_statement(p);
    }

    return n;
}

/**
 * Accepts a block element, which is either an expression or
 * an expression statement
 */
static struct ast_node *parser_acc_block_element(struct parser *p)
{
    struct ast_node *n;
    n = parser_acc_expression(p);
    if (n) {
        return n;
    }

    return parser_acc_expr_statement(p);
}

struct ast_node *parser_acc_block(struct parser *p, bool expect_braces)
{
    struct ast_node *n;
    struct token *tok;
    struct ast_node *element;
    const struct inplocation_mark *start = NULL;
    const struct inplocation_mark *end = NULL;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    if (expect_braces) {
        if (tok->type != TOKEN_SM_OCBRACE) {
            return NULL;
        }
        // consume '{'
        lexer_curr_token_advance(p->lexer);
        start = token_get_start(tok);
    }

    n = ast_block_create();
    if (!n) {
        RF_ERRNOMEM();
        return NULL;
    }

    //try to parse the first element
    element = parser_acc_block_element(p);
    if (!element) {
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
        lexer_curr_token_advance(p->lexer);

        ast_node_set_start(n, start);
        ast_node_set_end(n, token_get_end(tok));
        return n; //empty block
    }

    if (!start) {
        start = ast_node_startmark(element);
    }
    end = ast_node_endmark(element);
    ast_block_add_element(n, element);

    // now add elements to the block
    while ((element = parser_acc_block_element(p))) {
        ast_block_add_element(n, element);
        end = ast_node_endmark(element);
    }

    if (expect_braces) {
        tok = lexer_lookahead(p->lexer, 1);
        if (!tok || tok->type != TOKEN_SM_CCBRACE) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "Expected an expression or a '}' at block end");
            goto err_free_block;
        }
        //consume the '}'
        lexer_curr_token_advance(p->lexer);

        end = token_get_end(tok);
    }

    ast_node_set_start(n, start);
    ast_node_set_end(n, end);
    return n;

err_free_block:
        ast_node_destroy(n);
        return NULL;

}
