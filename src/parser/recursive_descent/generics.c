#include "generics.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/generics.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"


//failure in this means syntax error
static struct ast_node * parser_acc_genrtype(struct parser *p)
{
    struct ast_node *type_id;
    struct token *tok;
    struct ast_node *n;

    lexer_push(p->lexer);

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_IDENTIFIER) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected an identifier for the generic type kind");
        goto err;
    }
    type_id = token_get_value(tok);

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_IDENTIFIER) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected an identifier for the generic type name");
        ast_node_destroy(type_id);
        goto err;
    }


    lexer_pop(p->lexer);
    n = ast_genrtype_create(type_id, token_get_value(tok));
    if (!n) {
        //TODO: bad error
        return NULL;
    }
    return n;
err:
    lexer_rollback(p->lexer);
    return NULL;
}


static bool parser_acc_generic_decls_prime(struct parser *p,
                                           struct ast_node *parent)
{
    struct token *tok;
    struct ast_node *single;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_COMMA) {
        return true;
    }

    // consume ','
    lexer_next_token(p->lexer);
    single = parser_acc_genrtype(p);
    if (!single) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a generic type after ','");
        return false;
    }
    ast_node_add_child(parent, single);
    return parser_acc_generic_decls_prime(p, parent);
}

static bool parser_acc_generic_decls(struct parser *p,
                                      struct ast_node *parent)
{
    struct ast_node *single;

    single = parser_acc_genrtype(p);
    if (!single) {
        return false;
    }
    ast_node_add_child(parent, single);
    return parser_acc_generic_decls_prime(p, parent);
}

struct ast_node *parser_acc_genrdecl(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_LT) {
        return NULL;
    }
    // consume '<'
    lexer_next_token(p->lexer);

    n = ast_genrdecl_create(token_get_start(tok), NULL);
    if (!parser_acc_generic_decls(p, n)) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a generic declaration after '<'");
        ast_node_destroy(n);
        return NULL;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_OP_GT) {
        parser_synerr(
            p, token_get_start(tok), NULL,
            "Expected either a ',' or a '>' at generic declaration");
        ast_node_destroy(n);
        return NULL;
    }
    ast_node_set_end(n, token_get_end(tok));
    return n;
}

/* --- accepting a generic attribute --- */

static struct ast_node *parser_acc_genrattr_single(struct parser *p)
{
    struct ast_node *child;
    struct token *tok;
    lexer_push(p->lexer);

    tok = lexer_lookahead(p->lexer, 1);

    if (tok) {
        if (tok->type == TOKEN_SM_OPAREN) {
            lexer_next_token(p->lexer); // consume the OPAREN token

            child = parser_acc_typedesc(p);
            if (!child) {
                parser_synerr(p, token_get_end(tok), NULL,
                              "Expected a type description after '(' at generic"
                              " attribute");
                goto not_found;
            }
            tok = lexer_next_token(p->lexer);
            if (!tok || tok->type != TOKEN_SM_CPAREN) {
                parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                              "Expected a closing ')' at end of generic "
                              "attribute");
                goto err_free;
            }
        } else { // check for annotated identifier
            child = parser_acc_xidentifier(p);
            if (!child) {
                parser_synerr(
                    p, token_get_end(tok), NULL,
                    "Expected either a parenthesized type description or "
                    "an annotated identifier at generic attribute");
                goto not_found;
            }
        }
    } else {
        goto not_found;
    }

    //success
    lexer_pop(p->lexer);
    return child;


err_free:
    ast_node_destroy(child);
not_found:
    lexer_rollback(p->lexer);
    return NULL;
}


static bool parser_acc_generic_attribute_prime(struct parser *p,
                                               struct ast_node *parent)
{
    struct token *tok;
    struct ast_node *single;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_COMMA) {
        return true;
    }

    //consume ','
    lexer_next_token(p->lexer);
    single = parser_acc_genrattr_single(p);
    if (!single) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected a generic attribute after ','");
        return false;
    }
    ast_node_add_child(parent, single);
    return parser_acc_generic_attribute_prime(p, parent);
}

static bool parser_acc_generic_attribute(struct parser *p,
                                         struct ast_node *parent)
{
    struct ast_node *single;

    single = parser_acc_genrattr_single(p);
    if (!single) {
        return false;
    }
    ast_node_add_child(parent, single);
    return parser_acc_generic_attribute_prime(p, parent);
}

struct ast_node *parser_acc_genrattr(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_OP_LT) {
        return NULL;
    }
    //consume '<'
    lexer_next_token(p->lexer);
    n = ast_genrattr_create(token_get_start(tok), NULL);
    if (!n) {
        //TODO: bad error
        return NULL;
    }

    if (!parser_acc_generic_attribute(p, n)) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected generic attribute after '<'");
        ast_node_destroy(n);
        return NULL;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_OP_GT) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected '>' after generic attribute");
        ast_node_destroy(n);
        return NULL;
    }
    ast_node_set_end(n, token_get_end(tok));
    return n;
}
