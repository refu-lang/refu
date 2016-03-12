#include "type.h"

#include <Utils/sanity.h>

#include <ast/type.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include "common.h"
#include "identifier.h"

/**
 * Call to parse a parenthesized typedesc once you are sure that next token is '('
 */
static struct ast_node *ast_parser_acc_parenthesized_typedesc(struct ast_parser *p)
{
    struct ast_node *n;
    lexer_push(parser_lexer(p));
    //consume parentheses
    struct token *tok = lexer_curr_token_advance(parser_lexer(p));
    n = ast_parser_acc_typedesc_top(p);
    if (!n) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a type description after '('");
        goto fail;
    }

    tok = lexer_expect_token(parser_lexer(p), TOKEN_SM_CPAREN);
    if (!tok) {
        parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
                      "expected ')' after type description");
        goto fail_free_desc;
    }

    lexer_pop(parser_lexer(p));
    return n;

fail_free_desc:
    ast_node_destroy(n);
fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

struct ast_node *ast_parser_acc_typeleaf(struct ast_parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *tok2;
    struct ast_node *right;
    struct ast_node *left;
    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!XIDENTIFIER_START_COND(tok)) {
        return NULL;
    }
    lexer_push(parser_lexer(p));

    tok2 = lexer_lookahead(parser_lexer(p), 2);
    if (tok->type == TOKEN_IDENTIFIER &&
        tok2 && tok2->type == TOKEN_SM_COLON) {
        left = lexer_token_get_value(parser_lexer(p), tok);
        //consume identifier and ':'
        lexer_curr_token_advance(parser_lexer(p));
        lexer_curr_token_advance(parser_lexer(p));

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (tok->type == TOKEN_SM_OPAREN) {
            right = ast_parser_acc_parenthesized_typedesc(p);
        } else if (XIDENTIFIER_START_COND(tok)) {
            right = ast_parser_acc_xidentifier(p, true);
        } else {
            parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
                          "expected "XIDENTIFIER_START_STR" or '(' after ':'");
            goto err;
        }

        if (!right) {
            parser_synerr(p, lexer_last_token_end(parser_lexer(p)), NULL,
                          "expected "XIDENTIFIER_START_STR" or '(' after ':'");
            goto err;
        }

        n = ast_typeleaf_create(ast_node_startmark(left),
                                ast_node_endmark(right),
                                left, right);
        if (!n) {
            RF_ERRNOMEM();
            goto err;
        }
    } else { // last expansion of type_leaf rule, just an xidentifier
        n = ast_parser_acc_xidentifier(p, true);
    }

    lexer_pop(parser_lexer(p));
    return n;

err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

static struct ast_node *ast_parser_acc_typeelement(struct ast_parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok) {
        return NULL;
    }
    lexer_push(parser_lexer(p));

    if (tok->type == TOKEN_SM_OPAREN) {
        if (!(n = ast_parser_acc_parenthesized_typedesc(p))) {
            goto err;
        }
    } else {
        if (!(n = ast_parser_acc_typeleaf(p))) {
            goto err;
        }
    }

    lexer_pop(parser_lexer(p));
    return n;

err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

static struct ast_node *ast_parser_acc_typefactor_prime(
    struct ast_parser *p,
    struct ast_node *left_hand_side
)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_OP_COMMA) {
        return NULL;
    }
    lexer_push(parser_lexer(p));
    //consume comma
    lexer_curr_token_advance(parser_lexer(p));

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_PRODUCT, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }

    right_hand_side = ast_parser_acc_typeelement(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPEELEMENT_START_STR" after ','");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = ast_parser_acc_typefactor_prime(p, op);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free element, since it's freed inside prime accepting
        goto fail;
    }

    lexer_pop(parser_lexer(p));
    return ret ? ret : op;
fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

static struct ast_node *ast_parser_acc_typefactor(struct ast_parser *p)
{
    struct ast_node *element;
    struct ast_node *prime;

    element = ast_parser_acc_typeelement(p);
    if (!element) {
        return NULL;
    }
    prime = ast_parser_acc_typefactor_prime(p, element);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free element, since it's freed inside prime accepting
        return NULL;
    }

    return (prime) ? prime : element;
}

static struct ast_node *ast_parser_acc_typeterm_prime(
    struct ast_parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_OP_TYPESUM) {
        return NULL;
    }
    lexer_push(parser_lexer(p));
    //consume TYPESUM
    lexer_curr_token_advance(parser_lexer(p));

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_SUM, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }

    right_hand_side = ast_parser_acc_typefactor(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPEFACTOR_START_STR" after '|'");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = ast_parser_acc_typeterm_prime(p, op);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free term, since it's freed inside typeterm' accepting
        goto fail;
    }

    lexer_pop(parser_lexer(p));
    return ret ? ret : op;

fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

static struct ast_node *ast_parser_acc_typeterm(struct ast_parser *p)
{
    struct ast_node *factor;
    struct ast_node *prime;

    factor = ast_parser_acc_typefactor(p);
    if (!factor) {
        return NULL;
    }
    prime = ast_parser_acc_typeterm_prime(p, factor);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free factor, since it's freed inside typeterm' accepting
        return NULL;
    }

    return (prime) ? prime : factor;
}

static struct ast_node *ast_parser_acc_typedesc_prime(
    struct ast_parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_OP_IMPL) {
        return NULL;
    }
    lexer_push(parser_lexer(p));
    //consume IMPL
    lexer_curr_token_advance(parser_lexer(p));

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_IMPLICATION, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }
    right_hand_side = ast_parser_acc_typeterm(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPETERM_START_STR" after '->'");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = ast_parser_acc_typedesc_prime(p, op);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free op, since it's freed inside typedesc' accepting
        goto fail;
    }

    lexer_pop(parser_lexer(p));
    return ret ? ret : op;

fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

struct ast_node *ast_parser_acc_typedesc(struct ast_parser *p)
{
    struct ast_node *prime;
    struct ast_node *term;
    lexer_push(parser_lexer(p));
    term = ast_parser_acc_typeterm(p);
    if (!term) {
        goto fail;
    }

    prime = ast_parser_acc_typedesc_prime(p, term);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free term, since it's freed inside typedesc' accepting
        goto fail;
    }

    lexer_pop(parser_lexer(p));
    return prime ? prime : term;

fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

struct ast_node *ast_parser_acc_typedesc_top(struct ast_parser *p)
{
    struct ast_node *ret;
    struct ast_node *desc = ast_parser_acc_typedesc(p);
    if (!desc) {
        return NULL;
    }
    ret = ast_typedesc_create(desc);
    return ret;
}


struct ast_node *ast_parser_acc_typedecl(struct ast_parser *p)
{
    struct token *tok;
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *desc;
    const struct inplocation_mark *start;

    lexer_push(parser_lexer(p));

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_KW_TYPE) {
        goto not_found;
    }
    start = token_get_start(tok);

    name = ast_parser_acc_identifier(p);
    if (!name) {
        goto not_found;
    }

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_OCBRACE) {
        goto not_found;
    }

    desc = ast_parser_acc_typedesc_top(p);
    if (!desc) {
        parser_synerr(
            p, token_get_end(tok), NULL,
            "Expected data description for data declaration of \""RFS_PF"\"",
            RFS_PA(ast_identifier_str(name))
        );
        goto not_found;
    }

    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_typedecl_create(start, NULL, name, desc);
    if (!data_decl) {//memory error
        ast_node_destroy(desc);
        goto not_found;
    }

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_CCBRACE) {
        parser_synerr(
            p, lexer_last_token_end(parser_lexer(p)), NULL,
            "Expected a closing brace '}' in data declaration for '"RFS_PF"'",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free;
    }
    ast_node_set_end(data_decl, token_get_end(tok));

    lexer_pop(parser_lexer(p));
    return data_decl;

err_free:
    ast_node_destroy(data_decl);
not_found:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
