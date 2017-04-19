#include "typeclass.h"

#include <ast/typeclass.h>

#include <utils/common_strings.h>
#include <parser/parser.h>
#include "common.h"
#include "identifier.h"
#include "generics.h"
#include "function.h"

struct ast_node *ast_parser_acc_typeclass(struct ast_parser *p)
{
    struct ast_node *n = NULL;
    struct token *tok;
    struct ast_node *name;
    struct ast_node *genr = NULL;
    const struct inplocation_mark *start;
    enum parser_fndecl_list_err err;
    
    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_KW_TYPECLASS) {
        return NULL;
    }
    start = token_get_start(tok);
    lexer_push(parser_lexer(p));

    //consume typeclass keyword
    lexer_curr_token_advance(parser_lexer(p));

    name = ast_parser_acc_identifier(p);
    if (!name) {
        parser_synerr(
            p, lexer_last_token_start(parser_lexer(p)), NULL,
            "Expected an identifier for the typeclass name after 'class'"
        );
        goto err;
    }

    genr = ast_parser_acc_genrdecl(p);
    if (!genr && ast_parser_has_syntax_error_reset(p)) {
        parser_synerr(
            p, ast_node_endmark(name), NULL,
            "Expected a generic declaration for typeclass \""
            RFS_PF"\" after identifier",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free_genr;
    }

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_OCBRACE) {
        parser_synerr(
            p, ast_node_endmark(name), NULL,
            "Expected '{' at \""RFS_PF"\" typeclass "
            "declaration after identifier",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free_genr;
    }
    n = ast_typeclass_create(start, NULL, name, genr);
    if (!n) {
        RF_ERRNOMEM();
        goto err_free_genr;
    }

    err = ast_parser_acc_fndecl_list(p, n, FNDECL_PARTOF_TYPECLASS);
    switch (err) {
    case PARSER_FNDECL_LIST_EMPTY:
        parser_synerr(
            p, token_get_end(tok), NULL,
            "Expected at least one function declaration inside "
            "the body of typeclass \""RFS_PF"\" after '{'",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free_typeclass;
        break;
    case PARSER_FNDECL_LIST_FAILURE:
        parser_synerr(
            p, lexer_last_token_end(parser_lexer(p)), NULL,
            "Expected a proper function declaration inside "
            "typeclass \""RFS_PF"\"",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free_typeclass;
        break;
    default: // SUCCESS
        break;
    }

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_CCBRACE) {
        parser_synerr(
            p, lexer_last_token_end(parser_lexer(p)), NULL,
            "Expected '}' at the end of \""RFS_PF"\" typeclass declaration",
            RFS_PA(ast_identifier_str(name))
        );
        goto err_free_typeclass;
    }

    ast_node_set_end(n, token_get_end(tok));
    lexer_pop(parser_lexer(p));
    return n;

err_free_genr:
    if (genr) {
        ast_node_destroy(genr);
    }
err_free_typeclass:
    if (n) {
        ast_node_destroy(n);
    }
err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}

struct ast_node *ast_parser_acc_typeinstance(struct ast_parser *p)
{
    struct ast_node *n = NULL;
    struct token *tok;
    struct ast_node *class_name;
    struct ast_node *instance_name = NULL;
    struct ast_node *type_name;
    const struct inplocation_mark *start;
    enum parser_fnimpl_list_err err;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_KW_TYPEINSTANCE) {
        return NULL;
    }
    start = token_get_start(tok);
    lexer_push(parser_lexer(p));

    //consume typeclass instance keyword
    lexer_curr_token_advance(parser_lexer(p));

    class_name = ast_parser_acc_identifier(p);
    if (!class_name) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier for the typeclass instance "
            "class name after 'instance'"
        );
        goto err;
    }

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || (tok->type != TOKEN_IDENTIFIER && tok->type != TOKEN_KW_FOR)) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected either an identifier or 'for' after the typeinstance "
            "class name"
        );
        goto err;
    }

    instance_name = ast_parser_acc_identifier(p);
    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok || tok->type != TOKEN_KW_FOR) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a 'for' in the typeclass instance declaration before "
            "the name of the instantiated type"
        );
        goto err;
    }
    //consume for keyword
    lexer_curr_token_advance(parser_lexer(p));

    type_name = ast_parser_acc_identifier(p);
    if (!type_name) {
        parser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier for the name of the type that '"RFS_PF"' "
            "typeclass is instantiated with.",
            RFS_PA(ast_identifier_str(class_name))
        );
        goto err;
    }

    struct ast_node *default_identifier = ast_parser_peek_identifer(p);
    bool is_default = (
        default_identifier &&
        rf_string_equal(ast_identifier_str(default_identifier), &g_str_isdefault)
    );

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_OCBRACE) {
        parser_synerr(
            p, ast_node_endmark(type_name), NULL,
            "Expected '{' %sat type instance of '"RFS_PF"' after '"RFS_PF"'.",
            is_default ? "" : "or 'isdefault' ",
            RFS_PA(ast_identifier_str(class_name)),
            RFS_PA(ast_identifier_str(type_name))
        );
        goto err;
    }
    n = ast_typeinstance_create(
        start,
        NULL,
        class_name,
        instance_name,
        type_name,
        is_default
    );
    if (!n) {
        RF_ERRNOMEM();
        goto err;
    }

    err = ast_parser_acc_fnimpl_list(p, n);
    switch (err) {
    case PARSER_FNIMPL_LIST_EMPTY:
        parser_synerr(
            p, token_get_end(tok), NULL,
            "Expected at least one function implementation inside "
            "the body of type instance of '"RFS_PF"' for '"RFS_PF"' after '{'.",
            RFS_PA(ast_identifier_str(class_name)),
            RFS_PA(ast_identifier_str(type_name))
        );
        goto err_free_typeinstance;
        break;
    case PARSER_FNIMPL_LIST_FAILURE:
        parser_synerr(
            p, lexer_last_token_end(parser_lexer(p)), NULL,
            "Expected a proper function implementation inside "
            "the body of type instance of '"RFS_PF"' for '"RFS_PF"' after '{'.",
            RFS_PA(ast_identifier_str(class_name)),
            RFS_PA(ast_identifier_str(type_name))
        );
        goto err_free_typeinstance;
        break;
    default: // SUCCESS
        break;
    }

    tok = lexer_curr_token_advance(parser_lexer(p));
    if (!tok || tok->type != TOKEN_SM_CCBRACE) {
        parser_synerr(
            p, lexer_last_token_end(parser_lexer(p)), NULL,
            "Expected '}' at the end of \""RFS_PF"\" "
            "typeinstance for \""RFS_PF"\"",
            RFS_PA(ast_identifier_str(class_name)),
            RFS_PA(ast_identifier_str(type_name))
        );
        goto err_free_typeinstance;
    }
    ast_node_set_end(n, token_get_end(tok));

    return n;

err_free_typeinstance:
    if (n) {
        ast_node_destroy(n);
    }
err:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
