#include <parser/parser.h>

#include <inpfile.h>
#include <inpstr.h>
#include <ast/ast.h>
#include <lexer/lexer.h>

#include "common.h"
#include "type.h"
#include "function.h"
#include "block.h"
#include "vardecl.h"

static struct ast_node *parser_acc_stmt(struct parser *p);


bool parser_process_file(struct parser *p)
{
    struct ast_node *stmt;
    char *beg;

    beg = inpstr_beg(&p->file->str);
    p->root = ast_node_create_ptrs(
        AST_ROOT, p->file, beg,
        beg + inpstr_len_from_beg(&p->file->str));

    while ((stmt = parser_acc_stmt(p))) {
        ast_node_add_child(p->root, stmt);
    }

    if (NULL != lexer_next_token(p->lexer)) {
        parser_synerr(p, lexer_last_token_start(p->lexer), NULL,
                      "Expected a statement");
        return false;
    }

    return true;
}

static struct ast_node *parser_acc_stmt(struct parser *p)
{
    struct ast_node *stmt;

    lexer_push(p->lexer);
    if (!(stmt = parser_acc_block(p, true))) {
        goto fail;
    } else if ((stmt = parser_acc_vardecl(p))) {
        goto fail;
    } else if ((stmt = parser_acc_typedecl(p))) {
        goto fail;
    } else if ((stmt = parser_acc_fnimpl(p))) {
        goto fail;
    }

    lexer_pop(p->lexer);
    return stmt;
fail:
    lexer_rollback(p->lexer);
    return stmt;
}
