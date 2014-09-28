#include "expression.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"

static ast_node *parser_acc_expr_element(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }

    if (TOKEN_IS_NUMERIC_CONSTANT(tok)) {
        return ast_;
    }
}


struct ast_node *parser_acc_expression(struct parser *p)
{
}
