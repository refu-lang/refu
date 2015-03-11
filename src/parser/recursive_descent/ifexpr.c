#include "ifexpr.h"

#include <ast/ast.h>
#include <ast/ifexpr.h>
#include <info/info.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include "common.h"
#include "expression.h"
#include "block.h"

/* will always post a syntax error if it fails */
static struct ast_node *parser_acc_condbranch(struct parser *p,
                                              struct token *after_tok)
{
    struct ast_node *n;
    struct ast_node *expr;
    struct ast_node *block;

    expr = parser_acc_expression(p);
    if (!expr) {
        parser_synerr(p, token_get_end(after_tok), NULL,
                      "Expected an expression after '"RF_STR_PF_FMT"'",
                      RF_STR_PF_ARG(tokentype_to_str(after_tok->type)));
        return NULL;
    }

    block = parser_acc_block(p, true);
    if (!block) {
        parser_synerr(p, ast_node_endmark(expr), NULL,
                      "Expected a block after \""RF_STR_PF_FMT"\"'s "
                      "conditional expression",
                      RF_STR_PF_ARG(tokentype_to_str(after_tok->type)));
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

struct ast_node *parser_acc_ifexpr(struct parser *p, enum token_type if_type)
{
    struct ast_node *n;
    struct ast_node *branch;
    struct token *tok;
    const struct inplocation_mark *start;

    RF_ASSERT(if_type == TOKEN_KW_IF || if_type == TOKEN_KW_ELIF,
              "parse_ifexp called with invalid token type");
    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != if_type) {
        return NULL;
    }
    start = token_get_start(tok);
    // consume 'if' or 'elif'
    lexer_next_token(p->lexer);
    lexer_push(p->lexer);

    // parse the taken branch
    branch = parser_acc_condbranch(p, tok);
    if (!branch) {
        goto err;
    }

    // create the if expression
    n = ast_ifexpr_create(start, ast_node_endmark(branch),
                          branch, NULL);
    if (!n) {
        ast_node_destroy(branch);
        RF_ERRNOMEM();
        goto err;
    }

    tok = lexer_lookahead(p->lexer, 1);
    while (tok && (tok->type == TOKEN_KW_ELIF || tok->type == TOKEN_KW_ELSE)) {

        if (tok->type == TOKEN_KW_ELIF) {
            branch = parser_acc_ifexpr(p, TOKEN_KW_ELIF);
            if (!branch) {
                // error reporting should already happen in parser_acc_ifexpr()
                goto err_free;
            }
        } else { //can only be an else
            lexer_next_token(p->lexer); // consume it
            branch = parser_acc_block(p, true);
            if (!branch) {
                parser_synerr(p, token_get_end(tok), NULL,
                              "Expected a block after 'else'");
                goto err_free;
            }
        }

        ast_ifexpr_add_fallthrough_branch(n, branch);
        ast_node_set_end(n, ast_node_endmark(branch));
        tok = lexer_lookahead(p->lexer, 1);
    }

    lexer_pop(p->lexer);
    return n;

err_free:
    ast_node_destroy(n);
err:
    lexer_rollback(p->lexer);
    return NULL;
}
