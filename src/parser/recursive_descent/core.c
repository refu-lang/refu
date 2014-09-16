#include <parser/parser.h>

#include <inpfile.h>
#include <inpstr.h>
#include <ast/ast.h>
#include <lexer/lexer.h>

#include "common.h"
#include "type.h"
#include "function.h"

static struct ast_node *parser_acc_block(struct parser *p);
static struct ast_node *parser_acc_stmt(struct parser *p);
static struct ast_node *parser_acc_vardecl(struct parser *p);

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

/*
 * block = {
 */
static struct ast_node *parser_acc_block(struct parser *p)
{
#if 0
    struct ast_node *block;
    struct ast_node *stmt;
    block = ast_node_create(AST_BLOCK, );
    if (!block) {
        return NULL;
    }

    while (eof() || block_closes()) {
        stmt = parser_accept_statement(parser);
        if (!stmt) {
            //problem
        }
        rf_ilist_add(&block->children, &stmt->lh);
    }
#endif

    return NULL;
}


static struct ast_node *parser_acc_stmt(struct parser *p)
{
    struct ast_node *stmt;

    lexer_push(p->lexer);
    if (!(stmt = parser_acc_block(p))) {
        goto fail;
    } else if ((stmt = parser_acc_vardecl(p))) {
        goto fail;
    } else if ((stmt = parser_acc_typedecl(p))) {
        goto fail;
    } else if ((stmt = parser_acc_fndecl(p))) {
        goto fail;
    }

    lexer_pop(p->lexer);
    return stmt;
fail:
    lexer_rollback(p->lexer);
    return stmt;
}

static struct ast_node *parser_acc_vardecl(struct parser *p)
{
    return NULL;
//TODO
#if 0
    struct ast_node *var_decl;
    struct ast_node *id1;
    struct ast_node *id2;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_p(f);

    id1 = parser_file_acc_identifier(f);
    if (!id1) {
        goto not_found;
    }
    if(!parser_file_acc_string_ascii(f, &parser_tok_colon)) {
        goto not_found;
    }

    /* from here and on not having an identifier is an error */
    id2 = parser_file_acc_identifier(f);
    if (!id2) {
        parser_file_synerr(f, 0, "Expected an identifier");
        goto not_found;
    }
    ep = parser_file_p(f);

    var_decl = ast_vardecl_create(f, sp, ep, id1, id2);
    return var_decl;

not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
#endif
}
