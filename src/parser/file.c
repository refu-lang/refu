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


bool parser_file_eof(struct parser_file *f)
{
    parser_file_acc_ws(f);
    return rf_string_length(parser_file_str(f)) == 0;
}

void parser_file_move(struct parser_file *f,
                      unsigned int bytes,
                      unsigned int chars)
{
    struct parser_offset *off = &f->offset;
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    struct RFstring tmp;
    RF_STRING_SHALLOW_INIT(&tmp, parser_file_sp(f), bytes);

    off->bytes_moved += bytes;
    off->chars_moved += chars;
    //TODO: maybe have an argument of bytes in which to count occurences
    // for rf_string_count (?)
    off->lines_moved += rf_string_count(&tmp, &nl, 0);

    rf_stringx_move_bytes(parser_file_str(f), bytes);
}

void parser_file_move_to_offset(struct parser_file *f,
                                struct parser_offset *off)
{
    rf_stringx_move_to_index(parser_file_str(f), off->bytes_moved);
    parser_offset_copy(&f->offset, off);
}


void parser_file_acc_ws(struct parser_file *f)
{
    static const struct RFstring wsp = RF_STRING_STATIC_INIT(" \t\n\r");
    struct parser_offset mov;
    mov.chars_moved += rf_stringx_skip_chars(parser_file_str(f),
                                             &wsp,
                                             &mov.bytes_moved,
                                             &mov.lines_moved);
    parser_offset_add(&f->offset, &mov);
}

bool parser_file_acc_string_ascii(struct parser_file *f,
                                  const struct RFstring *str)
{
    struct RFstringx *buff = parser_file_str(f);

    if(!rf_string_begins_with(buff, str, 0)) {
        return false;
    }

    parser_file_move(f, str->length, str->length);
    return true;
}


i_INLINE_INS char *parser_file_sp(struct parser_file *f);
i_INLINE_INS struct RFstringx *parser_file_str(struct parser_file *f);
