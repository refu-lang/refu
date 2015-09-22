#include "module.h"

#include <parser/parser.h>
#include <lexer/lexer.h>

#include <ast/module.h>

#include "common.h"
#include "identifier.h"
#include "type.h"
#include "function.h"

struct ast_node *parser_acc_import(struct parser *p)
{
    struct ast_node *child;
    struct ast_node *import;
    const struct inplocation_mark *end = NULL;
    lexer_push(p->lexer);
    struct token *tok = lexer_lookahead(p->lexer, 1);
    if (!tok && (tok->type != TOKEN_KW_IMPORT || tok->type != TOKEN_KW_FOREIGN_IMPORT)) {
        return NULL;
    }
    // consume import
    lexer_next_token(p->lexer);

    import = ast_import_create(token_get_start(tok), NULL, tok->type == TOKEN_KW_FOREIGN_IMPORT);
    if (!import) {
        goto fail;
    }

    do {
        // try to parse a function declaration
        info_ctx_push(p->info);
        child = parser_acc_fndecl(p, FNDECL_PARTOF_FOREIGN_IMPORT);
        if (!child) {
            // if you can't try to parse only an identifier
            parser_info_rollback(p);
            child = parser_acc_identifier(p);
            if (!child) {
                parser_synerr(
                    p,
                    lexer_last_token_end(p->lexer),
                    NULL,
                    ast_import_is_foreign(import)
                    ? "Expected an identifier or a function declaration at foreign_import statement"
                    : "Expected an identifier at import statement"
                );
                goto fail_free;
            }
        } else {
            info_ctx_pop(p->info);
        }
        ast_node_add_child(import, child);
        end = ast_node_endmark(child);
    } while (lexer_expect_token(p->lexer, TOKEN_OP_COMMA));
    ast_node_set_end(import, end);

    lexer_pop(p->lexer);
    return import;

fail_free:
    ast_node_destroy(import);
fail:
    lexer_rollback(p->lexer);
    return NULL;
}

static struct ast_node *parser_acc_module_statement(struct parser *p)
{
    struct ast_node *stmt = NULL;
    struct token *tok = lexer_lookahead(p->lexer, 1);
    struct token *tok2 = lexer_lookahead(p->lexer, 2);

    if (TOKEN_IS_TYPEDECL_START(tok)) {
        stmt = parser_acc_typedecl(p);
    } else if (TOKEN_IS_IMPORT(tok)) {
        stmt = parser_acc_import(p);
    } else if (TOKENS_ARE_FNDECL_OR_IMPL(tok, tok2)) {
        stmt = parser_acc_fnimpl(p);
    }

    return stmt;
}

struct ast_node *parser_acc_module(struct parser *p)
{
    const struct inplocation_mark *start;
    struct token *tok;
    struct ast_node *n;
    struct ast_node *name;
    struct ast_node *args = NULL;
    struct ast_node *stmt;
    if (!(tok = lexer_expect_token(p->lexer, TOKEN_KW_MODULE))) {
        return NULL;
    }
    start = token_get_start(tok);
    lexer_push(p->lexer);

    name = parser_acc_identifier(p);
    if (!name) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected an identifier for the module name after 'module'");
        goto err;
    }

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || (tok->type != TOKEN_SM_OPAREN && tok->type != TOKEN_SM_OCBRACE)) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected a '(' or a '{' at module declaration");
        goto err;
    }

    if (tok->type == TOKEN_SM_OPAREN) {
        // consume '(' and parse the type description
        tok = lexer_next_token(p->lexer);
        args = parser_acc_typedesc(p);

        if (!args && parser_has_syntax_error_reset(p)) {
            parser_synerr(p, token_get_end(tok), NULL,
                          "Expected either a type description for the module's "
                          "arguments or ')' after '('");
            goto err;
        }

        if (!lexer_expect_token(p->lexer, TOKEN_SM_CPAREN)) {
            if (args) {
                parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                              "Expected ')' at module declaration after "
                              "type description");
            } else {
                parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                              "Expected ')' at module declaration after '('");
            }
            goto err_free_args;
        }
    }

    if (!lexer_expect_token(p->lexer, TOKEN_SM_OCBRACE)) {
       parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                     "Expected '{' at the beginning of module declaration");
       goto err_free_args;
    }


    if (!(n = ast_module_create(start, NULL, name, args))) {
        goto err_free_args;
    }
    args = NULL;

    while ((stmt = parser_acc_module_statement(p))) {
        ast_node_add_child(n, stmt);
    }

    if (parser_has_syntax_error_reset(p)) {
        // found an error during parsing a module statement
        goto err_free_module;
    }

    if (!(tok = lexer_expect_token(p->lexer, TOKEN_SM_CCBRACE))) {
        parser_synerr(p, lexer_last_token_end(p->lexer), NULL,
                      "Expected a module statement or '}'");
        goto err_free_module;
    }
    ast_node_set_end(n, token_get_end(tok));

    lexer_pop(p->lexer);
    return n;
    
err_free_module:
    ast_node_destroy(n);
err_free_args:
    if (args) {
        ast_node_destroy(args);
    }
err:
    lexer_rollback(p->lexer);
    return NULL;
}
