#include <parser/parser.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/tokens.h>
#include <parser/function.h>
#include <parser/type.h>

static struct ast_node * parser_file_acc_block(struct parser_file *f);
static struct ast_node *parser_file_acc_stmt(struct parser_file *f);

struct parser_ctx *parser_new()
{
    struct parser_ctx *ret;
    RF_MALLOC(ret, sizeof(struct parser_ctx), return NULL);

    rf_ilist_head_init(&ret->files);
    ret->current_file = NULL;

    return ret;
}

void parser_flush_messages(struct parser_ctx *parser)
{
    struct parser_file *f;
    rf_ilist_for_each(&parser->files, f, lh) {
        info_ctx_flush(f->info, stdout, MESSAGE_ANY);
    }
}

/*
 * block = {
 */
static struct ast_node *parser_file_acc_block(struct parser_file *f)
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

static bool parser_begin_parsing(struct parser_ctx *parser,
                                 struct parser_file *file)
{
    struct ast_node *stmt;
    struct parser_offset off = PARSER_OFFSET_STATIC_INIT();
    char *beg = parser_string_beg(&file->pstr);

    file->root = ast_node_create(AST_ROOT, file, beg,
                                 beg + parser_string_len_from_beg(&file->pstr));

    while (stmt = parser_file_acc_stmt(file)) {
        ast_node_add_child(file->root, stmt);
    }
    if (!parser_file_eof(file)) {
        parser_file_synerr(file, "Expected a statement");
        return false;
    }

    return true;
}

bool parser_process_file(struct parser_ctx *parser, const struct RFstring *name)
{
    struct parser_file *file = parser_file_new(name);
    if (!file) {
        ERROR("Could not open file \""RF_STR_PF_FMT"\"", RF_STR_PF_ARG(name));
        return false;
    }

    rf_ilist_add(&parser->files, &file->lh);
    parser->current_file = file;

    return parser_begin_parsing(parser, file);
}


static struct ast_node *parser_file_acc_stmt(struct parser_file *f)
{
    struct ast_node *stmt;
    struct parser_offset proff;
    parser_offset_copy(&proff, &f->offset);

    if (stmt = parser_file_acc_block(f)) {
        return stmt;
    } else if (stmt = parser_file_acc_vardecl(f)) {
        return stmt;
    } else if (stmt = parser_file_acc_typedecl(f)) {
        return stmt;
    } else if (stmt = parser_file_acc_fndecl(f)) {
        return stmt;
    }

    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_vardecl(struct parser_file *f)
{
    struct ast_node *var_decl;
    struct ast_node *id1;
    struct ast_node *id2;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

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
        parser_file_synerr(f, "Expected an identifier");
        goto not_found;
    }
    ep = parser_file_sp(f);

    var_decl = ast_vardecl_create(f, sp, ep, id1, id2);
    return var_decl;

not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}



struct ast_node *parser_file_acc_identifier(struct parser_file *f)
{
    struct parser_offset proff;
    char *p;
    char *sp;
    char *ep;
    char *lim;
    bool first_char = false;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = p = parser_file_sp(f);
    lim = parser_file_sp(f) + rf_string_length_bytes(parser_file_str(f)) - 1;

    if (lim - p <= 0) { /* if already at the end do nothing */
        goto end;
    }

    if ((*p >= 'A' && *p <= 'Z') ||
        (*p >= 'a' && *p <= 'z')) {
        p ++;
    } else {
        goto end;
    }

    while (p <= lim) {
        if ((*p >= 'A' && *p <= 'Z') ||
            (*p >= 'a' && *p <= 'z') ||
            (*p >= '0' && *p <= '9')) {
            if (p < lim) { /* don't go over the limit */
                p ++;
            } else {
                break;
            }
            continue;
        }
        break;
    }

    if (p == sp) { /* no identifier was found */
        goto end;
    }
    ep = p;

    parser_file_move(f, p - sp, p - sp);
    return ast_identifier_create(f, sp, ep);

end:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}
