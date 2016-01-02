#include <parser/parser.h>

#include <inpfile.h>
#include <module.h>
#include <inpstr.h>
#include <ast/function.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <lexer/lexer.h>
#include <front_ctx.h>
#include <compiler.h>
#include <utils/common_strings.h>

#include "common.h"
#include "type.h"
#include "function.h"
#include "block.h"
#include "vardecl.h"
#include "module.h"

static struct ast_node *parser_acc_stmt(struct parser *p);

static bool do_finalize_parsing(struct ast_node *n, void* user_arg)
{
    bool *main_found = user_arg;
    n->state = AST_NODE_STATE_AFTER_PARSING;
    if (n->type == AST_FUNCTION_DECLARATION) {
        if (rf_string_equal(ast_fndecl_name_str(n), &g_str_main)) {
            *main_found = true;
        }
    }
    return true;
}

bool parser_finalize_parsing(struct parser *p)
{
    bool main_found = false;
    ast_pre_traverse_tree(p->root, do_finalize_parsing, &main_found);
    // if this is the main module or something else set the front's main flag
    if (main_found || p->front->is_main) {
        if (!compiler_set_main(p->front)) {
            return false;
        }
        if (!module_create(p->root, p->front)) {
            return false;
        }
    }
    return true;
}

bool parser_parse_file(struct parser *p)
{
    struct ast_node *stmt;
    p->root = ast_root_create(p->file);
    while ((stmt = parser_acc_stmt(p))) {
        ast_node_add_child(p->root, stmt);
    }

    if (NULL != lexer_curr_token_advance(p->lexer)) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected an outermost statement");
        return false;
    }
    return true;
}

bool parser_process_file(struct parser *p)
{
    if (!parser_parse_file(p)) {
        return false;
    }
    if (!parser_finalize_parsing(p)) {
        return false;
    }
    return true;
}

static struct ast_node *parser_acc_stmt(struct parser *p)
{
    struct token *tok2;
    struct token *tok;
    struct ast_node *stmt = NULL;

    tok = lexer_lookahead(p->lexer, 1);
    tok2 = lexer_lookahead(p->lexer, 2);

    // TODO: Maybe change these, since each one of these macros actually checks for token existence too
    if (TOKEN_IS_MODULE_START(tok)) {
        stmt = parser_acc_module(p);
        if (!stmt || !module_create(stmt, p->front)) {
            return NULL;
        }
    } else if (TOKEN_IS_BLOCK_START(tok)) {
        stmt = parser_acc_block(p, true);
    } else if (TOKENS_ARE_POSSIBLE_VARDECL(tok, tok2)) {
        stmt = parser_acc_vardecl(p);
    } else if (TOKEN_IS_TYPEDECL_START(tok)) {
        stmt = parser_acc_typedecl(p);
    } else if (TOKENS_ARE_FNDECL_OR_IMPL(tok, tok2)) {
        stmt = parser_acc_fnimpl(p);
    } else if (TOKEN_IS_IMPORT(tok)) {
        stmt = parser_acc_import(p);
    }

    return stmt;
}
