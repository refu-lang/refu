#include "function.h"

#include <ast/function.h>

#include <parser/parser.h>
#include "common.h"
#include "identifier.h"
#include "generics.h"
#include "type.h"
#include "expression.h"
#include "block.h"

struct ast_node *parser_acc_fndecl(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *oparen_tok;
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
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected an identifier for the function name after 'fn'");
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
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected '(' at function declaration");
        goto err_free_genr;
    }
    oparen_tok = tok;

    args = parser_acc_typedesc(p);
    if (!args && parser_has_syntax_error_reset(p)) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected either a type description for the function's "
                      "arguments or ')' after '('");
        goto err_free_genr;
    }

    tok = lexer_next_token(p->lexer);
    if (!tok || tok->type != TOKEN_SM_CPAREN) {
        if (args) {
            parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                          "Expected ')' at function declaration after "
                          "type description");
        } else {
            parser_synerr(p, token_get_end(oparen_tok), NULL,
                          "Expected ')' at function declaration after '('");
        }
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
                          "Expected type description for the function's "
                          "return type after '->'");
            goto err_free_args;
        }
        end = ast_node_endmark(ret_type);
    }

    n = ast_fndecl_create(start, end, name, genr, args, ret_type);
    if (!n) {
        RF_ERRNOMEM();
        goto err_free_rettype;
    }
    return n;

err_free_rettype:
    if (ret_type) {
        ast_node_destroy(ret_type);
    }
err_free_args:
    if (args) {
    ast_node_destroy(args);
    }
err_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
err_free_name:
    ast_node_destroy(name);
err:
    return NULL;
}


enum parser_fndecl_list_err  parser_acc_fndecl_list(struct parser *p,
                                                    struct ast_node *parent)
{
    struct ast_node *decl;
    struct token *tok;
    tok = lexer_lookahead(p->lexer, 1);

    if (!tok || tok->type != TOKEN_KW_FUNCTION) {
        return PARSER_FNDECL_LIST_EMPTY;
    }

    while (tok && tok->type == TOKEN_KW_FUNCTION) {
        decl = parser_acc_fndecl(p);
        if (!decl) {
            return PARSER_FNDECL_LIST_FAILURE;
        }
        ast_node_add_child(parent, decl);
        tok = lexer_lookahead(p->lexer, 1);
    }

    return PARSER_FNDECL_LIST_SUCCESS;
}

struct ast_node *parser_acc_fnimpl(struct parser *p)
{
    struct ast_node *n;
    struct ast_node *decl;
    struct ast_node *body;

    decl = parser_acc_fndecl(p);
    if (!decl) {
        return NULL;
    }

    body = parser_acc_block(p, true);
    if (!body) {
        parser_synerr(p, ast_node_endmark(decl), NULL,
                      "Expected a body for \""RF_STR_PF_FMT"\" function "
                      "implementation", RF_STR_PF_ARG(ast_fndecl_name_str(decl)));
        ast_node_destroy(decl);
        return NULL;
    }

    n = ast_fnimpl_create(ast_node_startmark(decl), ast_node_endmark(body),
                          decl, body);
    if (!n) {
        RF_ERRNOMEM();
        ast_node_destroy(body);
        ast_node_destroy(decl);
        return NULL;
    }
    return n;
}

enum parser_fnimpl_list_err parser_acc_fnimpl_list(struct parser *p,
                                                   struct ast_node *parent)
{
   struct ast_node *impl;
   struct token *tok;
   tok = lexer_lookahead(p->lexer, 1);

   if (!tok || tok->type != TOKEN_KW_FUNCTION) {
       return PARSER_FNIMPL_LIST_EMPTY;
   }

   while (tok && tok->type == TOKEN_KW_FUNCTION) {
       impl = parser_acc_fnimpl(p);
       if (!impl) {
           return PARSER_FNIMPL_LIST_FAILURE;
       }
       ast_node_add_child(parent, impl);
       tok = lexer_lookahead(p->lexer, 1);
   }

   return PARSER_FNIMPL_LIST_SUCCESS;
}

struct ast_node *parser_acc_fncall(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct ast_node *name;
    struct ast_node *genr = NULL;

    name = parser_acc_identifier(p);
    if (!name) {
        return NULL;
    }

    genr = parser_acc_genrattr(p);
    if (!genr && parser_has_syntax_error(p)) {
        goto err_free_name;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_SM_OPAREN) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected '('");
        goto err_free_genr;
    }
    //consume '('
    lexer_next_token(p->lexer);

    n = ast_fncall_create(ast_node_startmark(name), NULL, name, genr);
    if (!n) {
        RF_ERRNOMEM();
        goto err_free_genr;
    }

    if (!parser_acc_expressions_list(p, n)) {
        ast_node_destroy(n);
        return NULL;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || tok->type != TOKEN_SM_CPAREN) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected ')' at end of "RF_STR_PF_FMT" function call",
                      RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free_genr;
    }
    //consume ')'
    lexer_next_token(p->lexer);
    ast_node_set_end(n, token_get_end(tok));

    return n;

err_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
err_free_name:
    ast_node_destroy(name);
    return NULL;
}
