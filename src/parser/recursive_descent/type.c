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
static struct ast_node *parser_acc_parenthesized_typedesc(struct parser *p)
{
    struct ast_node *n;
    lexer_push(p->lexer);
    //consume parentheses
    struct token *tok = lexer_next_token(p->lexer);
    n = parser_acc_typedesc_top(p);
    if (!n) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a type description after '('");
        goto fail;
    }

    tok = lexer_expect_token(p->lexer, TOKEN_SM_CPAREN);
    if (!tok) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "expected ')' after type description");
        goto fail_free_desc;
    }

    lexer_pop(p->lexer);
    return n;

fail_free_desc:
    ast_node_destroy(n);
fail:
    lexer_rollback(p->lexer);
    return NULL;
}

struct ast_node *parser_acc_typeleaf(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *tok2;
    struct ast_node *right;
    struct ast_node *left;
    tok = lexer_lookahead(p->lexer, 1);
    if (!XIDENTIFIER_START_COND(tok)) {
        return NULL;
    }
    lexer_push(p->lexer);

    tok2 = lexer_lookahead(p->lexer, 2);
    if (tok->type == TOKEN_IDENTIFIER &&
        tok2 && tok2->type == TOKEN_SM_COLON) {
        left = token_get_value(tok);
        //consume identifier and ':'
        lexer_next_token(p->lexer);
        lexer_next_token(p->lexer);

        tok = lexer_lookahead(p->lexer, 1);
        if (tok->type == TOKEN_SM_OPAREN) {
            right = parser_acc_parenthesized_typedesc(p);
        } else if (XIDENTIFIER_START_COND(tok)) {
            right = parser_acc_xidentifier(p, true);
        } else {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "expected "XIDENTIFIER_START_STR" or '(' after ':'");
            goto err;
        }

        if (!right) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
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
        n = parser_acc_xidentifier(p, true);
    }

    lexer_pop(p->lexer);
    return n;

err:
    lexer_rollback(p->lexer);
    return NULL;
}

static struct ast_node *parser_acc_typeelement(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }
    lexer_push(p->lexer);

    if (tok->type == TOKEN_SM_OPAREN) {
        if (!(n = parser_acc_parenthesized_typedesc(p))) {
            goto err;
        }
    } else {
        if (!(n = parser_acc_typeleaf(p))) {
            goto err;
        }
    }

    lexer_pop(p->lexer);
    return n;

err:
    lexer_rollback(p->lexer);
    return NULL;
}

static struct ast_node *parser_acc_typefactor_prime(
    struct parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_COMMA) {
        return NULL;
    }
    lexer_push(p->lexer);
    //consume comma
    lexer_next_token(p->lexer);

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_PRODUCT, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }

    right_hand_side = parser_acc_typeelement(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPEELEMENT_START_STR" after ','");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = parser_acc_typefactor_prime(p, op);
    if (parser_has_syntax_error(p)) {
        // no need to free element, since it's freed inside prime accepting
        goto fail;
    }

    lexer_pop(p->lexer);
    return ret ? ret : op;
fail:
    lexer_rollback(p->lexer);
    return NULL;
}

static struct ast_node *parser_acc_typefactor(struct parser *p)
{
    struct ast_node *element;
    struct ast_node *prime;

    element = parser_acc_typeelement(p);
    if (!element) {
        return NULL;
    }
    prime = parser_acc_typefactor_prime(p, element);
    if (parser_has_syntax_error(p)) {
        // no need to free element, since it's freed inside prime accepting
        return NULL;
    }

    return (prime) ? prime : element;
}

static struct ast_node *parser_acc_typeterm_prime(
    struct parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_TYPESUM) {
        return NULL;
    }
    lexer_push(p->lexer);
    //consume TYPESUM
    lexer_next_token(p->lexer);

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_SUM, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }

    right_hand_side = parser_acc_typefactor(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPEFACTOR_START_STR" after '|'");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = parser_acc_typeterm_prime(p, op);
    if (parser_has_syntax_error(p)) {
        // no need to free term, since it's freed inside typeterm' accepting
        goto fail;
    }

    lexer_pop(p->lexer);
    return ret ? ret : op;

fail:
    lexer_rollback(p->lexer);
    return NULL;
}

static struct ast_node *parser_acc_typeterm(struct parser *p)
{
    struct ast_node *factor;
    struct ast_node *prime;

    factor = parser_acc_typefactor(p);
    if (!factor) {
        return NULL;
    }
    prime = parser_acc_typeterm_prime(p, factor);
    if (parser_has_syntax_error(p)) {
        // no need to free factor, since it's freed inside typeterm' accepting
        return NULL;
    }

    return (prime) ? prime : factor;
}

static struct ast_node *parser_acc_typedesc_prime(
    struct parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_IMPL) {
        return NULL;
    }
    lexer_push(p->lexer);
    //consume IMPL
    lexer_next_token(p->lexer);

    op = ast_typeop_create(ast_node_startmark(left_hand_side), NULL,
                           TYPEOP_IMPLICATION, left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        goto fail;
    }
    right_hand_side = parser_acc_typeterm(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a "TYPETERM_START_STR" after '->'");
        ast_node_destroy(op);
        goto fail;
    }
    ast_typeop_set_right(op, right_hand_side);

    ret = parser_acc_typedesc_prime(p, op);
    if (parser_has_syntax_error(p)) {
        // no need to free op, since it's freed inside typedesc' accepting
        goto fail;
    }

    lexer_pop(p->lexer);
    return ret ? ret : op;

fail:
    lexer_rollback(p->lexer);
    return NULL;
}

struct ast_node *parser_acc_typedesc(struct parser *p)
{
    struct ast_node *prime;
    struct ast_node *term;
    lexer_push(p->lexer);
    term = parser_acc_typeterm(p);
    if (!term) {
        goto fail;
    }

    prime = parser_acc_typedesc_prime(p, term);
    if (parser_has_syntax_error(p)) {
        // no need to free term, since it's freed inside typedesc' accepting
        goto fail;
    }

    lexer_pop(p->lexer);
    return prime ? prime : term;

fail:
    lexer_rollback(p->lexer);
    return NULL;
}

struct ast_node *parser_acc_typedesc_top(struct parser *p)
{
    struct ast_node *ret;
    struct ast_node *desc = parser_acc_typedesc(p);
    if (!desc) {
        return NULL;
    }
    ret = ast_typedesc_create(desc);
    return ret;
}


struct ast_node *parser_acc_typedecl(struct parser *p)
{
    struct token *tok;
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *desc;
    const struct inplocation_mark *start;

    lexer_push(p->lexer);

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_KW_TYPE) {
        goto not_found;
    }
    start = token_get_start(tok);

    name = parser_acc_identifier(p);
    if (!name) {
        goto not_found;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_OCBRACE) {
        goto not_found;
    }

    desc = parser_acc_typedesc_top(p);
    if (!desc) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected data description for data declaration "
                      "of \""RF_STR_PF_FMT"\"",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto not_found;
    }

    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_typedecl_create(start, NULL, name, desc);
    if (!data_decl) {//memory error
        ast_node_destroy(desc);
        goto not_found;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_CCBRACE) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected a closing brace '}' in data "
                      "declaration for '"RF_STR_PF_FMT"'",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }
    ast_node_set_end(data_decl, token_get_end(tok));

    lexer_pop(p->lexer);
    return data_decl;

err_free:
    ast_node_destroy(data_decl);
not_found:
    lexer_rollback(p->lexer);
    return NULL;
}
