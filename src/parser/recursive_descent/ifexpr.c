#include "ifexpr.h"

#include <ast/ast.h>
#include <ast/ifexpr.h>
#include <info/info.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include "common.h"
#include "expression.h"
#include "block.h"

static struct ast_node *parser_acc_condbranch(struct parser *p)
{
    struct ast_node *n;
    struct ast_node *expr;
    struct ast_node *block;

    expr = parser_acc_expression(p);
    if (!expr) {
        return NULL;
    }

    block = parser_acc_block(p, true);
    if (!block) {
        parser_synerr(p, ast_node_endmark(expr), NULL,
                      "expected a block after conditional expression");
        ast_node_destroy(expr);
        return NULL;
    }

    n = ast_condbranch_create(ast_node_startmark(expr), 
                              ast_node_endmark(block),
                              expr, block);
    if (!n) {
        RF_ERRNOMEM();
        ast_node_destroy(expr);
        ast_node_destroy(block);
        return NULL;
    }
    
    return n;
}

struct ast_node *parser_acc_ifexpr(struct parser *p)
{
    struct ast_node *n;
    struct ast_node *taken_branch;
    struct token *tok;
    
    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_KW_IF) {
        return NULL;
    }

    // consume 'if'
    lexer_next_token(p->lexer);
    
    taken_branch = parser_acc_condbranch(p);
    if (!taken_branch) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "expected a condition expression after 'if'");
    }
    
    // TODO: the rest of the parsing
    return n;
}
