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
    printf("PARSED:\n"RF_STR_PF_FMT, RF_STR_PF_ARG(&file->buffer));
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
