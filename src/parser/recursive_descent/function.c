#include "function.h"

#include <ast/function.h>

#include <parser/parser.h>
#include "common.h"
#include "identifier.h"
#include "generics.h"
#include "type.h"

struct ast_node *parser_acc_fndecl(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct ast_node *name;
    struct ast_node *genr = NULL;
    struct ast_node *args = NULL;
    struct ast_node *ret_type = NULL;
    struct inplocation_mark *start;
    struct inplocation_mark *end;

    tok = lexer_lookahead(p->lexer, 1);

    if (!tok || tok->type != TOKEN_KW_FUNCTION) {
        return NULL;
    }
    start = token_get_start(tok);

    //consume function keyword
    lexer_next_token(p->lexer);

    name = parser_acc_identifier(p);
    if (!name) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "expected an identifier for the function name after 'fn'");
        goto err;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (GENRDECL_START_COND(tok)) {
        genr = parser_acc_genrdecl(p);
        if (!genr) {
            goto err_free_name;
        }
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_OPAREN) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "expected '(' at function declaration");
        goto err_free_genr;
    }

    args = parser_acc_typedesc(p);
    if (!args && parser_has_syntax_error_reset(p)) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "expected type description for the function's "
                      "arguments after '('");
        goto err_free_genr;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_CPAREN) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "expected ')' at function declaration");
        goto err_free_args;
    }
    end = token_get_end(tok);

    tok = lexer_lookahead(p->lexer, 1);
    if (tok && tok->type == TOKEN_OP_IMPL) {
        //consume '->'
        lexer_next_token(p->lexer);
        ret_type = parser_acc_typedesc(p);
        if (!ret_type) {
            parser_synerr(p, token_get_end(tok), NULL,
                          "expected type description for the function's "
                          "return type after '->'");
            goto err_free_args;
        }
        end = ast_node_endmark(ret_type);
    }

    n = ast_fndecl_create(start, end, name, genr, args, ret_type);
    if (!n) {
        //TODO: Bad error
        goto err_free_rettype;
    }
    return n;

err_free_rettype:
    if (ret_type) {
        ast_node_destroy(ret_type);
    }
err_free_args:
    ast_node_destroy(args);
err_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
err_free_name:
    ast_node_destroy(name);
err:
    return NULL;
}
