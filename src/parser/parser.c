#include <parser/parser.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/tokens.h>

static struct ast_node * parser_file_acc_block(struct parser_file *f);
static struct ast_node *parser_file_acc_stmt(struct parser_file *f);
static struct ast_node *parser_file_acc_vardecl(struct parser_file *f);
static struct ast_node *parser_file_acc_datadecl(struct parser_file *f);
static struct ast_node *parser_file_acc_fndecl(struct parser_file *f);
static struct ast_node *parser_file_acc_identifier(struct parser_file *f);

struct parser_ctx *parser_new()
{
    struct parser_ctx *ret;
    RF_MALLOC(ret, sizeof(struct parser_ctx), NULL);

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
    struct parser_offset off = PARSER_OFFSET_INIT();
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
    } else if (stmt = parser_file_acc_datadecl(f)) {
        return stmt;
    } else if (stmt = parser_file_acc_fndecl(f)) {
        return stmt;
    }

    parser_file_move_to_offset(f, &proff);
    return NULL;
}

static struct ast_node *parser_file_acc_datadecl(struct parser_file *f)
{
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *member;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_tok_data)) {
        goto not_found;
    }

    name = parser_file_acc_identifier(f);
    if (!name) {
        goto not_found;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_ocbrace)) {
        goto not_found;
    }

    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_datadecl_create(f, sp, NULL, name);

    while ((member = parser_file_acc_vardecl(f)) != NULL) {
        ast_datadecl_add_member(data_decl, member);
    }
    if (parser_file_has_synerr(f)) {
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing brace '}' in data "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_ccbrace)) {
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing brace '}' in data "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }

    ast_node_set_end(data_decl, parser_file_sp(f));

    return data_decl;

err_free:
    ast_datadecl_destroy(data_decl);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

static struct ast_node *parser_file_acc_vardecl(struct parser_file *f)
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

static struct ast_node *parser_file_acc_fndecl(struct parser_file *f)
{
    bool found_comma;
    struct ast_node *fn;
    struct ast_node *name;
    struct ast_node *arg;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_tok_fn)) {
        goto not_found;
    }

    /* error from here and on */
    parser_file_acc_ws(f);
    name = parser_file_acc_identifier(f);
    if (!name) {
        goto not_found;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
        ast_node_destroy(name);
        goto not_found;
    }

    fn = ast_fndecl_create(f, sp, NULL, name);

    while ((arg = parser_file_acc_vardecl(f)) != NULL) {
        ast_fndecl_add_arg(fn, arg);
        found_comma = false;
        parser_file_acc_ws(f);
        if (!parser_file_acc_string_ascii(f, &parser_tok_comma)) {
            found_comma = true;
        }
    }
    if (parser_file_has_synerr(f)) {
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing parentheses ')' function "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }
    
    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_cparen)) {    
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing parentheses ')' function "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_arrow)) {
        /* no return value */
        ast_node_set_end(fn, parser_file_sp(f));
        return fn;
    }

    parser_file_acc_ws(f);
    name = parser_file_acc_identifier(f);
    if (!name) {
        parser_file_synerr(f,
                           "Expected a return type for function  "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(fn->fndecl.name)));
        goto err_free;
    }

    ast_fndecl_set_ret(fn, name);
    ast_node_set_end(fn, parser_file_sp(f));
    return fn;

err_free:
    ast_node_destroy(fn);
not_found:
    return NULL;
}

static struct ast_node *parser_file_acc_identifier(struct parser_file *f)
{
    struct parser_offset proff;
    char *p;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = p = parser_file_sp(f);
    ep = parser_file_sp(f) + rf_string_length_bytes(parser_file_str(f));

    while (p <= ep) {
        if ((*p >= 'A' && *p <= 'Z') ||
            (*p >= 'a' && *p <= 'z')) {
            p ++;
            continue;
        }
        break;
    }
    ep = p;

    if (ep == sp) { /* no identifier was found */
        return NULL;
    }

    parser_file_move(f, p - sp, p - sp);
    return ast_identifier_create(f, sp, ep);
}
