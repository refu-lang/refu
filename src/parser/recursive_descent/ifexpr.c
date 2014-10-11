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
    struct ast_node *branch;
    struct token *tok;
    struct inplocation_mark *start;
    
    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_KW_IF) {
        return NULL;
    }
    start = token_get_start(tok);

    // consume 'if'
    lexer_next_token(p->lexer);
    
    // parse the taken branch
    branch = parser_acc_condbranch(p);
    if (!branch) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "expected a conditional branch after 'if'");
        return NULL;
    }


    // create the if expression
    n = ast_ifexpr_create(start, ast_node_endmark(branch),
                          branch, NULL);
    if (!n) {
        ast_node_destroy(branch);
        RF_ERRNOMEM();
        return NULL;
    }

    tok = lexer_lookahead(p->lexer, 1);
    while (tok && (tok->type == TOKEN_KW_ELIF || tok->type == TOKEN_KW_ELSE)) {
        // consume 'elif' or 'else'
        lexer_next_token(p->lexer);

        if (tok->type == TOKEN_KW_ELIF) {
            branch = parser_acc_condbranch(p);
            if (!branch) {
                parser_synerr(p, token_get_end(tok), NULL,
                              "expected a conditional branch after 'elif'");
                ast_node_destroy(n);
                return NULL;
            }
            ast_ifexpr_add_elif_branch(n, branch);

        } else { //can only be an else
            branch = parser_acc_block(p, true);
            if (!branch) {
                parser_synerr(p, token_get_end(tok), NULL,
                              "expected a block after 'else'");
                ast_node_destroy(n);
                return NULL;
            }
            ast_ifexpr_add_fall_through_branch(n, branch);
        }

        ast_node_set_end(n, ast_node_endmark(branch));
        tok = lexer_lookahead(p->lexer, 1);
    }
    
    return n;
}
