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

    RF_MALLOC(ret, sizeof(*ret), return NULL);

    ret->info = info_ctx_create();
    if (!ret->info) {
        free(ret);
        return NULL;
    }

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
    parser_offset_init(&ret->offset);

    rf_array_deinit(&lines_arr);
    return ret;
}

void parser_file_deinit(struct parser_file *f)
{
    if (f->root) {
        ast_node_destroy(f->root);
    }
    rf_string_deinit(&f->file_name);
    parser_string_deinit(&f->pstr);
    info_ctx_destroy(f->info);
}


bool parser_file_eof(struct parser_file *f)
{
    struct parser_offset proff;
    parser_offset_copy(&proff, &f->offset);
    parser_file_acc_ws(f);
    if (rf_string_length(parser_file_str(f)) == 0) {
        return true;
    }

    parser_file_move_to_offset(f, &proff);
    return false;
}

char *parser_file_lookfor(struct parser_file *f,
                          const struct RFstring *str,
                          char *end)
{
    int32_t ret;
    struct RFstringx *s = parser_file_str(f);
    ret = rf_string_find_i(s, str, 0, end - rf_string_data(s), 0);
    if (ret == RF_FAILURE) {
        return NULL;
    }

    return parser_file_sp(f) + ret;
}

void parser_file_move(struct parser_file *f,
                      unsigned int bytes,
                      unsigned int chars)
{
    struct parser_offset *off = &f->offset;
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    uint32_t lim = parser_string_len_from_beg(&f->pstr);

    // don't go over file limit
    if (off->bytes_moved + bytes > lim) {
        RF_ASSERT(0); //should never request such a big move
    }

    off->bytes_moved += bytes;
    off->chars_moved += chars;
    off->lines_moved += rf_string_count(parser_file_str(f), &nl, bytes, 0, 0);

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
    struct parser_offset mov = PARSER_OFFSET_STATIC_INIT();
    mov.chars_moved = rf_stringx_skip_chars(parser_file_str(f),
                                            &wsp,
                                            0,
                                            &mov.bytes_moved,
                                            &mov.lines_moved);

    parser_offset_add(&f->offset, &mov);
}

bool parser_file_acc_string_ascii(struct parser_file *f,
                                  const struct RFstring *str)
{
    char *lim;
    struct RFstringx *buff = parser_file_str(f);
    lim = parser_file_sp(f) + rf_string_length_bytes(buff) - 1;
    if(!rf_string_begins_with(buff, str, 0)) {
        return false;
    }

    parser_file_move(f, str->length, str->length);
    return true;
}

bool parser_file_line(struct parser_file *f,
                      uint32_t line,
                      struct RFstring *str)
{
    struct parser_string *s = &f->pstr;
    unsigned int end;
    if (line + 1 > s->lines_num) {
        return false;
    }
    if (line + 1 == s->lines_num) {
        end = parser_string_len_from_beg(s);
    } else {
        end = s->lines[line + 1];
    }

    RF_STRING_SHALLOW_INIT(str,
                           parser_string_beg(s) + s->lines[line],
                           end - s->lines[line]);
    return true;
}

i_INLINE_INS bool parser_file_has_synerr(struct parser_file *f);
i_INLINE_INS char *parser_file_sp(struct parser_file *f);
i_INLINE_INS struct RFstringx *parser_file_str(struct parser_file *f);
i_INLINE_INS struct parser_offset *parser_file_offset(struct parser_file *f);
