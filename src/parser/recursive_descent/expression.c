#include "expression.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"

static struct ast_node *parser_acc_expr_element(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    if (TOKEN_IS_NUMERIC_CONSTANT(tok) ||
        tok->type == TOKEN_IDENTIFIER ||
        tok->type == TOKEN_STRING_LITERAL) {
        n = token_get_value(tok);
    } else {
        parser_synerr(p, token_get_start(tok), NULL,
                      "expected either an identifier, a numeric constant, "
                      "a function call or an array reference");
        return NULL;
    }
    //consume the token and return
    lexer_next_token(p->lexer);
    return n;
}


struct ast_node *parser_acc_expression(struct parser *p)
{
    // TODO: implement properly
    return parser_acc_expr_element(p);
}


bool parser_acc_expressions_list(struct parser *p,
                                 struct ast_node *parent)
{
    struct ast_node *expr;
    struct token *tok;

    // check for the first expression
    expr = parser_acc_expression(p);

    if (!expr) { // empty list is valid
        return true;
    }
    ast_node_add_child(parent, expr);

    tok = lexer_lookahead(p->lexer, 1);
    while (tok && tok->type == TOKEN_OP_COMMA) {
        // consume the comma
        lexer_next_token(p->lexer);
        // get the next expression
        expr = parser_acc_expression(p);
        if (!expr) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected an expression after ','");
            return false;
        }
        ast_node_add_child(parent, expr);
        tok = lexer_lookahead(p->lexer, 1);
    }

    return true;
}
