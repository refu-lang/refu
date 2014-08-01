#include <parser.h>

#include <RFmemory.h>
#include <RFtextfile.h>

#include <messaging.h>

static struct parser_file *parser_file_new(const struct RFstring *name)
{
    struct parser_file *ret;
    struct RFtextfile file;

    RF_MALLOC(ret, sizeof(struct parser_file), NULL);
    if (!rf_string_copy_in(&ret->file_name, name)) {
        return NULL;
    }

    if (!rf_textfile_init(&file, name,
                          RF_FILE_READ, RF_ENDIANESS_UNKNOWN,
                          RF_UTF8, RF_EOL_AUTO)) {
        return NULL;
    }
    if (-1 == rf_textfile_read_lines(&file, 0, &ret->buffer)) {
        return NULL;
    }
    rf_textfile_deinit(&file);

    ret->current_line = 0;
    ret->current_col = 0;

    return ret;
}


struct parser_ctx *parser_new()
{
    struct parser_ctx *ret;
    RF_MALLOC(ret, sizeof(struct parser_ctx), NULL);

    rf_ilist_head_init(&ret->files);
    ret->current_file = NULL;
    return ret;
}




static bool parser_begin_parsing(struct parser_ctx *parser,
                                 struct parser_file *file)
{
    struct ast_node *root;
    struct ast_node *stmt;
    struct parser_offset off = PARSER_OFFSET_INIT();

    /* printf("PARSED:\n"RF_STR_PF_FMT, RF_STR_PF_ARG(&file->buffer)); */
    root = ast_node_new(AST_ROOT, file, 0, 0, rf_string_data(file->buffer));

    while (stmt = parser_accept_statement(parser, &off)) {
        ast_node_add_child(root, stmt);
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

static struct ast_node *parser_accept_statement(struct parser_ctx *parser)
{
    struct ast_node *stmt;
    struct parser_offset proff;
    parser_offset_copy(&proff, PARSER_COFF(parser));


    if (stmt = parser_accept_block(parser)) {
        return stmt;
    } else if (stmt = parser_accept_variable_declaration(parser)) {
        return stmt;
    }

    parser_move_to_offset(parser, &proff);
    return NULL;
}

static struct ast_node *parser_accept_variable_declaration(
    struct parser_ctx *parser)
{
    struct ast_node *var_decl;
    struct ast_node *id1;
    struct ast_node *id2;
    struct parser_offset proff;
    char *beg;
    char *end;
    static const struct RFstring colon = RF_STRING_STATIC_INIT(":");
    
    parser_offset_copy(&proff, PARSER_COFF(parser));

    parser_accept_ws(parser);
    beg = PARSER_CBUFF(parser);
    
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
    end = PARSER_CBUFF(parser);
    
    var_decl = ast_node_new(AST_VARIABLE_DECLARATION,
                            parser->current_file,
                            beg, end);
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
    struct ast_node *identifier;
    struct parser_offset proff;
    char *beg;
    char *end;
    char *p;

    parser_offset_copy(&proff, PARSER_COFF(parser));

    parser_accept_ws(parser);
    beg = p = PARSER_CBUFF(parser);
    end = PARSER_CBUFF(parser) + rf_string_length_bytes(parser->current_file->buffer);

    while (p <= end) {
        if ((*p >= 'A' && *p <= 'Z') ||
            (*p >= 'a' && *p <= 'z')) {
            p ++;
            continue;
        }
        break;
    }
    end = p;
    
    identifier = ast_node_new(AST_VARIABLE_DECLARATION, parser->current_file,
                              beg, end);
    if (!identifier) {
        //TODO: memory error
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&identifier->value_identifier, beg, end - beg);
    return identifier;
}

/*
 * block = {
 */
static bool parser_accept_block(struct parser_ctx *parser)
{
    struct ast_node *block;
    struct ast_node *stmt;
    block = ast_node_new(AST_BLOCK, );
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

    return true;
}

i_INLINE_INS void parser_move_to_offset(struct parser_ctx *parser,
                                         struct parser_offset *off);
