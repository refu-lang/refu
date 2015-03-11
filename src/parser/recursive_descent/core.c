#include <parser/parser.h>

#include <inpfile.h>
#include <inpstr.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <lexer/lexer.h>

#include "common.h"
#include "type.h"
#include "function.h"
#include "block.h"
#include "vardecl.h"

static struct ast_node *parser_acc_stmt(struct parser *p);

static bool do_finalize_parsing(struct ast_node *n, void* user_arg)
{
    (void) user_arg;
    n->state = AST_NODE_STATE_AFTER_PARSING;
    return true;
}

void parser_finalize_parsing(struct ast_node *n)
{
    ast_pre_traverse_tree(n, do_finalize_parsing, NULL);
}

bool parser_process_file(struct parser *p)
{
    struct ast_node *stmt;
    p->root = ast_root_create(p->file);

    while ((stmt = parser_acc_stmt(p))) {
        ast_node_add_child(p->root, stmt);
    }

    if (NULL != lexer_next_token(p->lexer)) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected a statement");
        return false;
    }

    // at the end of parsing let's signify that all of the nodes are owned by the parser
    parser_finalize_parsing(p->root);
    return true;
}

static struct ast_node *parser_acc_stmt(struct parser *p)
{
    struct ast_node *stmt = NULL;
    struct token *tok;
    struct token *tok2;

    tok = lexer_lookahead(p->lexer, 1);
    tok2 = lexer_lookahead(p->lexer, 2);

    if (TOKEN_IS_BLOCK_START(tok)) {
        stmt = parser_acc_block(p, true);
    } else if (TOKENS_ARE_POSSIBLE_VARDECL(tok, tok2)) {
        stmt = parser_acc_vardecl(p);
    } else if (TOKEN_IS_TYPEDECL_START(tok)) {
        stmt = parser_acc_typedecl(p);
    } else if (TOKENS_ARE_FNDECL_OR_IMPL(tok, tok2)) {
        stmt = parser_acc_fnimpl(p);
    }

    return stmt;
}
