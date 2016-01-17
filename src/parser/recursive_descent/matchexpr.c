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

static struct ast_node *ast_parser_accept_matchcase(struct ast_parser *p)
{
    struct token *tok;
    struct ast_node *pattern = ast_parser_acc_typedesc_top(p);
    struct ast_node *expr;
    struct ast_node *n;
    if (!pattern) {
        return NULL;
    }

    tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_THICKARROW);
    if (!tok) {
        parser_synerr(p, ast_node_endmark(pattern), NULL,
                      "Expected '=>' after match pattern");
        goto fail_free_pattern;
    }

    expr = ast_parser_acc_expression(p);
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

struct ast_node *ast_parser_acc_matchexpr(struct ast_parser *p,
                                          bool have_header,
                                          bool expect_it)
{
    struct token *tok;
    struct ast_node *id;
    struct ast_node *match_case;
    struct ast_node *match_expr;
    const struct inplocation_mark *start;
    bool got_paren = false;
    lexer_push(parser_lexer(p));

    if (have_header) {
        tok = lexer_expect_token(parser_lexer(p), TOKEN_KW_MATCH);
        if (!tok) {
            if (expect_it) {
                parser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                              "Expected 'match' keyword");
            }
            goto fail;
        }
        start = token_get_start(tok);

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (tok && tok->type == TOKEN_SM_OPAREN) {
            lexer_curr_token_advance(parser_lexer(p));
            got_paren = true;
        }
        id = ast_parser_acc_identifier(p);
        if (!id) {
            parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
                          "Expected an identifier after 'match' keyword");
            goto fail;
        }
        if (got_paren && !(tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_CPAREN))) {
            parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
                          "Expected a closing ')' after identifier");
            goto fail;
        }
        match_expr = ast_matchexpr_create(start, token_get_end(tok), id);
        if (!match_expr) {
            RF_ERROR("Failed to allocate a match expression node");
            goto fail;
        }
        
        tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_OCBRACE);
        if (!tok) { // a body-less match expression. Can only appear inside another match expression
            return match_expr;
        }
    } else {
        // match expression as body of a function
        if (!(match_case = ast_parser_accept_matchcase(p))) {
            if (expect_it) {
                parser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                              "Expected headless match expression");
            }
            goto fail;
        }
        match_expr = ast_matchexpr_create(ast_node_startmark(match_case),
                                          ast_node_endmark(match_case),
                                          NULL);
        if (!match_expr) {
            ast_node_destroy(match_case);
            RF_ERROR("Failed to allocate a match expression node");
            goto fail;
        }
        ast_matchexpr_add_case(match_expr, match_case);
    }

    const struct inplocation_mark *end;
    while ((match_case = ast_parser_accept_matchcase(p))) {
        ast_matchexpr_add_case(match_expr, match_case);
        end = ast_node_endmark(match_case);
    }
    if (ast_parser_has_syntax_error(p)) { // last match_case parsing failed
        goto fail_free_matchexpr;
    }

    if (have_header) {
        // if the match expression has a header we expect a curly brace at the end
        tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_CCBRACE);
        if (!tok) {
            parser_synerr(p, ast_node_endmark(id), NULL,
                          "Expected a '}' after match case");
            goto fail_free_matchexpr;
        }
        end = token_get_end(tok);
    }
    ast_node_set_end(match_expr, end);
    lexer_pop(parser_lexer(p));
    return match_expr;

fail_free_matchexpr:
    ast_node_destroy(match_expr);
fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

