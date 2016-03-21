#include "arr.h"

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <ast/arr.h>

#include "common.h"
#include "expression.h"

static bool ast_parser_acc_array_single(
    struct ast_parser *p,
    struct ast_node **ret,
    struct inplocation_mark *end
)
{
    *ret = NULL;
    // consume '['
    lexer_curr_token_advance(parser_lexer(p));
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    RF_ASSERT(tok, "Check existence of '[' before calling this function");
    if (tok->type == TOKEN_SM_CSBRACE) {
        // no expression in array specifier, consume ']'
        lexer_curr_token_advance(parser_lexer(p));
        *end = *token_get_end(tok);
        return true;
    }
    *ret = ast_parser_acc_expression(p);
    if (!*ret) {
        if (ast_parser_has_syntax_error(p)) {
            parser_synerr(
                p, lexer_last_token_start(parser_lexer(p)), NULL,
                "Expected expression inside array brackets"
            );
            return false;
        }
    }
    if (!(tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_CSBRACE))) {
            parser_synerr(
                p, lexer_last_token_end(parser_lexer(p)), NULL,
                "Expected ']' to close the array bracket"
            );
        goto fail;
    }
    *end = *token_get_end(tok);
    return true;

fail:
    if (*ret) {
        ast_node_destroy(*ret);
    }
    return false;
}

struct ast_node *ast_parser_acc_arrspec(struct ast_parser *p)
{
    struct token *tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_SM_OSBRACE) {
        return NULL;
    }
    lexer_push(parser_lexer(p));

    struct ast_node **it_expr;
    struct arr_ast_nodes dimensions;
    const struct inplocation_mark *start = token_get_start(tok);
    struct inplocation_mark end;
    darray_init(dimensions);
    do {
        struct ast_node *expr;
        if (!ast_parser_acc_array_single(p, &expr, &end)) {
            goto err;
        }
        darray_append(dimensions, expr);
        tok = lexer_lookahead(parser_lexer(p), 1);
    } while(tok && tok->type == TOKEN_SM_OSBRACE);

    struct ast_node *n = ast_arrspec_create(start, &end, &dimensions);
    if (!n) {
        goto err;
    }
    lexer_pop(parser_lexer(p));
    return n;

err:
    darray_foreach(it_expr, dimensions) {
        if (*it_expr) {
            ast_node_destroy(*it_expr);
        }
    }
    lexer_rollback(parser_lexer(p));
    return NULL;
}
