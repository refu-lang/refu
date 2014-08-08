#include <parser/file.h>

#include <RFmemory.h>
#include <RFtextfile.h>
#include <Utils/array.h>

#include <ast/ast.h>

#define PARSER_STRING_STARTING_LINES 256 // TODO: move somewhere else?
#define FILE_BUFFER_INITIAL_SIZE 1024

struct parser_file *parser_file_new(const struct RFstring *name)
{
    struct parser_file *ret;
    struct RFtextfile file;
    struct RFstringx file_str;
    struct RFarray lines_arr;
    int lines;
    RF_ARRAY_TEMP_INIT(&lines_arr, uint32_t, PARSER_STRING_STARTING_LINES);

    RF_MALLOC(ret, sizeof(*ret), NULL);

    if (!rf_stringx_init_buff(&file_str, FILE_BUFFER_INITIAL_SIZE, "")) {
        return NULL;
    }
    if (!rf_string_copy_in(&ret->file_name, name)) {
        return NULL;
    }

    if (!rf_textfile_init(&file, name,
                          RF_FILE_READ, RF_ENDIANESS_UNKNOWN,
                          RF_UTF8, RF_EOL_AUTO)) {
        return NULL;
    }

    lines = rf_textfile_read_lines(&file, 0, &file_str, &lines_arr);
    if (lines == -1) {
        return NULL;
    }
    rf_textfile_deinit(&file);

    if (!parser_string_init(&ret->pstr, &file_str, &lines_arr, lines)) {
        return NULL;
    }

    ret->current_line = 0;
    ret->current_col = 0;

    rf_array_deinit(&lines_arr);
    return ret;
}

void parser_file_deinit(struct parser_file *f)
{
    ast_node_destroy(f->root);
    rf_string_deinit(&f->file_name);
    parser_string_deinit(&f->pstr);
}

