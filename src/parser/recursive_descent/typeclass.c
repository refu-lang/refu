#include "typeclass.h"

#include <ast/typeclass.h>

#include <parser/parser.h>
#include "common.h"
#include "identifier.h"
#include "generics.h"
#include "function.h"

struct ast_node *parser_acc_typeclass(struct parser *p)
{
    struct ast_node *n = NULL;
    struct token *tok;
    struct ast_node *name;
    struct ast_node *genr = NULL;
    struct inplocation_mark *start;
    enum parser_fndecl_list_err err;

    tok = lexer_lookahead(p->lexer, 1);

    if (!tok || tok->type != TOKEN_KW_TYPECLASS) {
        return NULL;
    }
    start = token_get_start(tok);

    //consume typeclass keyword
    lexer_next_token(p->lexer);

    name = parser_acc_identifier(p);
    if (!name) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected an identifier for the typeclass "
                      "name after 'class'");
        goto err;
    }


    genr = parser_acc_genrdecl(p);
    if (!genr && parser_has_syntax_error_reset(p)) {
        parser_synerr(p, ast_node_endmark(name), NULL,
                      "Expected a generic declaration for typeclass \""
                      RF_STR_PF_FMT"\" after identifier",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_name;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_OCBRACE) {
        parser_synerr(p, ast_node_endmark(name), NULL,
                      "Expected '{' at \""RF_STR_PF_FMT"\" typeclass "
                      "declaration after identifier",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_genr;
    }
    n = ast_typeclass_create(start, NULL, name, genr);
    if (!n) {
        RF_ERRNOMEM();
        goto err_free_genr;
    }

    err = parser_acc_fndecl_list(p, n);
    switch (err) {
    case PARSER_FNDECL_LIST_EMPTY:
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected at least one function declaration inside "
                      "the body of typeclass \""RF_STR_PF_FMT"\" after '{'",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_typeclass;
        break;
    case PARSER_FNDECL_LIST_FAILURE:
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected a proper function declaration inside "
                      "typeclass \""RF_STR_PF_FMT"\"",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_typeclass;
        break;
    default: // SUCCESS
        break;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_CCBRACE) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected '}' at the end of \""RF_STR_PF_FMT"\" "
                      "typeclass declaration",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_typeclass;
    }
    ast_node_set_end(n, token_get_end(tok));

    return n;

err_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
err_free_name:
    ast_node_destroy(name);
err_free_typeclass:
    if (n) {
        ast_node_destroy(n);
    }
err:
    return NULL;
}
