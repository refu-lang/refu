#include <parser/parser.h>

#include <messaging.h>
#include <ast/ast.h>
#include <ast/identifier.h>

static struct ast_node * parser_accept_block(struct parser_ctx *parser);
static struct ast_node *parser_accept_statement(struct parser_ctx *parser);
static struct ast_node *parser_accept_vardecl(struct parser_ctx *parser);
static struct ast_node *parser_accept_identifier(struct parser_ctx *parser);

struct parser_ctx *parser_new()
{
    struct parser_ctx *ret;
    RF_MALLOC(ret, sizeof(struct parser_ctx), NULL);

    rf_ilist_head_init(&ret->files);
    ret->current_file = NULL;
    return ret;
}

/*
 * block = {
 */
static struct ast_node * parser_accept_block(struct parser_ctx *parser)
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

static struct ast_node *parser_accept_statement(struct parser_ctx *parser)
{
    struct ast_node *stmt;
    struct parser_offset proff;
    parser_offset_copy(&proff, parser_curr_off(parser));


    if (stmt = parser_accept_block(parser)) {
        return stmt;
    } else if (stmt = parser_accept_vardecl(parser)) {
        return stmt;
    }

    parser_move_to_offset(parser, &proff);
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

    while (stmt = parser_accept_statement(parser)) {
        ast_node_add_child(file->root, stmt);
    }

    return true;
}

bool parser_process_file(struct parser_ctx *parser, const struct RFstring *name)
{
    struct parser_file *file = parser_file_new(name);
    if (!file) {
        print_error("Could not open file %s", name);
        return false;
    }

    rf_ilist_add(&parser->files, &file->lh);
    parser->current_file = file;

    return parser_begin_parsing(parser, file);
}

static struct ast_node *parser_accept_vardecl(struct parser_ctx *parser)
{
    struct ast_node *var_decl;
    struct ast_node *id1;
    struct ast_node *id2;
    struct parser_offset proff;
    char *sp;
    char *ep;
    static const struct RFstring colon = RF_STRING_STATIC_INIT(":");
    parser_offset_copy(&proff, parser_curr_off(parser));

    parser_accept_ws(parser);
    sp = parser_curr_sp(parser);

    id1 = parser_accept_identifier(parser);
    if (!id1) {
        parser_move_to_offset(parser, &proff);
        return NULL;
    }
    if(!parser_accept_string_ascii(parser, &colon)) {
        parser_move_to_offset(parser, &proff);
        return NULL;
    }
    id2 = parser_accept_identifier(parser);
    if (!id2) {
        parser_move_to_offset(parser, &proff);
        return NULL;
    }
    ep = parser_curr_sp(parser);

    var_decl = ast_node_create(AST_VARIABLE_DECLARATION,
                               parser->current_file, sp, ep);
    if (!var_decl) {
        //TODO: memory error
        return NULL;
    }
    ast_node_add_child(var_decl, id1);
    ast_node_add_child(var_decl, id2);

    return var_decl;
}

static struct ast_node *parser_accept_identifier(struct parser_ctx *parser)
{
    struct parser_offset proff;
    char *p;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, parser_curr_off(parser));

    parser_accept_ws(parser);
    sp = p = parser_curr_sp(parser);
    ep = parser_curr_sp(parser) + rf_string_length_bytes(parser_curr_str(parser));

    while (p <= ep) {
        if ((*p >= 'A' && *p <= 'Z') ||
            (*p >= 'a' && *p <= 'z')) {
            p ++;
            continue;
        }
        break;
    }
    ep = p;

    return ast_identifier_create(parser->current_file, sp, ep);
}

i_INLINE_INS void parser_move_to_offset(struct parser_ctx *parser,
                                         struct parser_offset *off);
i_INLINE_INS void parser_move(struct parser_ctx *parser,
                               unsigned int bytes,
                               unsigned int chars);
i_INLINE_INS void parser_accept_ws(struct parser_ctx *parser);
i_INLINE_INS bool parser_accept_string_ascii(struct parser_ctx *parser,
                                              const struct RFstring *str);
i_INLINE_INS struct RFstringx *parser_curr_str(struct parser_ctx *p);
i_INLINE_INS struct parser_string *parser_curr_pstr(struct parser_ctx *p);
i_INLINE_INS char *parser_curr_sp(struct parser_ctx *p);
i_INLINE_INS struct parser_offset *parser_curr_off(struct parser_ctx *p);


