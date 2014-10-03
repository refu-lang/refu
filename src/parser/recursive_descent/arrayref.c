#include "arrayref.h"

#include <ast/arrayref.h>

#include <parser/parser.h>
#include "common.h"
#include "identifier.h"
#include "expression.h"

struct ast_node *parser_acc_arrayref(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct ast_node *name;
    struct ast_node *expr;

    name = parser_acc_identifier(p);
    if (!name) {
        return NULL;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_SM_OSBRACE) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected '['");
        goto err_free_name;
    }
    //consume '['
    lexer_next_token(p->lexer);

    expr = parser_acc_expression(p);
    if (!expr) {
        parser_synerr(p, token_get_start(tok), NULL,
                      "Expected expression after '['");
        goto err_free_name;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_SM_CSBRACE) {
        parser_synerr(p, ast_node_endmark(expr), NULL,
                      "Expected ']' at end of "RF_STR_PF_FMT" array reference",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_expr;
    }
    //consume ']'
    lexer_next_token(p->lexer);

    n = ast_arrayref_create(ast_node_startmark(name), token_get_end(tok),
                            name, expr);

    if (!n) {
        //TODO: bad error
        goto err_free_expr;
        return NULL;
    }
    return n;

err_free_expr:
    ast_node_destroy(expr);
err_free_name:
    ast_node_destroy(name);
    return NULL;
}
