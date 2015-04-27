#include "matchexpr.h"

#include <ast/ast.h>
#include <ast/matchexpr.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include <inplocation.h>

#include "common.h"
#include "identifier.h"
#include "expression.h"
#include "type.h"

static struct ast_node *parser_accept_matchcase(struct parser *p)
{
    struct token *tok;
    struct ast_node *pattern = parser_acc_typedesc_top(p);
    struct ast_node *expr;
    struct ast_node *n;
    if (!pattern) {
        return NULL;
    }

    tok = lexer_expect_token(p->lexer, TOKEN_SM_THICKARROW);
    if (!tok) {
        parser_synerr(p, ast_node_endmark(pattern), NULL,
                      "Expected '=>' after match pattern");
        goto fail_free_pattern;
    }

    expr = parser_acc_expression(p);
    if (!expr) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected an expression after '=>'");
        goto fail_free_pattern;
    }
    
    n = ast_matchcase_create(ast_node_startmark(pattern),
                         ast_node_endmark(expr),
                         pattern,
                         expr);
    if (!n) {
        goto fail_free_expr;
    }
    return n;

fail_free_expr:
    ast_node_destroy(expr);
fail_free_pattern:
    ast_node_destroy(pattern);
    return NULL;
}

struct ast_node *parser_acc_matchexpr(struct parser *p, bool expect_it)
{
    struct token *tok;
    struct ast_node *id;
    struct ast_node *match_case;
    struct ast_node *match_expr;
    const struct inplocation_mark *start;
    bool got_paren = false;
    tok = lexer_expect_token(p->lexer, TOKEN_KW_MATCH);
    if (!tok) {
        if (expect_it) {
            parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                          "Expected 'match' keyword");
        }
        return NULL;
    }
    start = token_get_start(tok);

    tok = lexer_lookahead(p->lexer, 1);
    if (tok && tok->type == TOKEN_SM_OPAREN) {
        lexer_next_token(p->lexer);
        got_paren = true;
    }
    id = parser_acc_identifier(p);
    if (!id) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected an identifier after 'match' keyword");
        return NULL;
    }
    if (got_paren && !(tok = lexer_expect_token(p->lexer, TOKEN_SM_CPAREN))) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected a closing ')' after identifier");
        return NULL;
    }

    /* match_expr = ast_matchexpr_create(start, lexer_last_token_end(p->lexer), id); */
    match_expr = ast_matchexpr_create(start, token_get_end(tok), id);
    if (!match_expr) {
        RF_ERROR("Failed to allocate a match expression node");
        return NULL;
    }

    tok = lexer_expect_token(p->lexer, TOKEN_SM_OCBRACE);
    if (!tok) { // a body-less match expression. Can only appear inside another match expression
        return match_expr;
    }

    while ((match_case = parser_accept_matchcase(p))) {
        ast_matchexpr_add_case(match_expr, match_case);
    }
    if (parser_has_syntax_error(p)) { // last match_case parsing failed
        goto fail_free_matchexpr;
    }

    tok = lexer_expect_token(p->lexer, TOKEN_SM_CCBRACE);
    if (!tok) {
        parser_synerr(p, ast_node_endmark(id), NULL,
                      "Expected a '}' after match case");
        goto fail_free_matchexpr;
    }
    ast_node_set_end(match_expr, token_get_end(tok));
    return match_expr;

fail_free_matchexpr:
    ast_node_destroy(match_expr);
    return NULL;
}

