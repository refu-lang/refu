#include <inpfile.h>

#include <rflib/io/rf_textfile.h>
#include <rflib/string/rf_str_core.h>
#include <rflib/string/rf_str_traversalx.h>

#include <ast/ast.h>
#include <inpstr.h>
#include <unistd.h>

static bool inpfile_init(struct inpfile* f,
                         const struct RFstring *name,
                         const struct RFstring *contents)
{
    struct RFstringx file_str;
    struct RFarray lines_arr;
    unsigned int lines;
    RF_STRUCT_ZERO(f);
    RF_ARRAY_TEMP_INIT(&lines_arr, uint32_t, INPUT_STRING_STARTING_LINES);

    if (!rf_string_copy_in(&f->file_name, name)) {
        RF_ERRNOMEM();
        return false;
    }

    if (!contents) { // if we read from a file
        if (!rf_textfile_tostr_in(name, &lines, &lines_arr, &file_str)) {
            goto fail_free_name;
        }

        if (!inpstr_init(&f->str, &file_str, &lines_arr, lines)) {
            goto fail_free_filecase_stringbuff;
        }

    } else { //we read string contents directly
        if (!inpstr_init_from_source(&f->str, contents)) {
            goto fail_free_name;
        }
    }


    inpoffset_init(&f->offset);
    rf_array_deinit(&lines_arr);
    return true;

fail_free_filecase_stringbuff:
    rf_stringx_deinit(&file_str);
fail_free_name:
    rf_string_deinit(&f->file_name);
    return false;
}

struct inpfile *inpfile_create(const struct RFstring *name)
{
    struct inpfile *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!inpfile_init(ret, name, NULL)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct inpfile *inpfile_create_from_string(const struct RFstring *name,
                                           const struct RFstring *contents)
{
    struct inpfile *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!inpfile_init(ret, name, contents)) {
        free(ret);
        ret = NULL;
    }
    return ret;    
}

void inpfile_deinit(struct inpfile *f)
{
    if (f->root) {
        ast_node_destroy(f->root);
    }
    rf_string_deinit(&f->file_name);
    inpstr_deinit(&f->str);
}

void inpfile_destroy(struct inpfile *f)
{
    inpfile_deinit(f);
    free(f);
}

void inpfile_acc_ws(struct inpfile *f)
{
    static const struct RFstring wsp = RF_STRING_STATIC_INIT(" \t\n\r");
    struct inpoffset mov = INPOFFSET_STATIC_INIT();
    mov.chars_moved = rf_stringx_skip_chars(inpfile_str(f),
                                            &wsp,
                                            0,
                                            &mov.bytes_moved,
                                            &mov.lines_moved);

    inpoffset_add(&f->offset, &mov);
}

void inpfile_move(struct inpfile *f,
                  unsigned int bytes,
                  unsigned int chars)
{
    struct inpoffset *off = &f->offset;
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    uint32_t lim = inpstr_len_from_beg(&f->str);

    RF_ASSERT_OR_EXIT(off->bytes_moved + bytes <= lim,
                      "Requested parsed file move over limit");

    off->bytes_moved += bytes;
    off->chars_moved += chars;
    off->lines_moved += rf_string_count(RF_STRX2STR(inpfile_str(f)), &nl, bytes, 0, 0);

    rf_stringx_move_bytes(inpfile_str(f), bytes);
}

bool inpfile_line(struct inpfile *f,
                  uint32_t line,
                  struct RFstring *str)
{
    struct inpstr *s = &f->str;
    unsigned int end;
    if (line == s->lines_num) {
        return false;
    }
    if (line + 1 == s->lines_num) {
        end = inpstr_len_from_beg(s);
    } else {
        end = s->lines[line + 1];
    }

    RF_STRING_SHALLOW_INIT(str,
                           inpstr_beg(s) + s->lines[line],
                           end - s->lines[line]);
    return true;
}

char *inpfile_line_p(struct inpfile *f, unsigned int line)
{
    struct inpstr *s = &f->str;
    if (line == s->lines_num) {
        return NULL;
    }

    return inpfile_sp(f) + s->lines[line];
}


i_INLINE_INS char *inpfile_p(struct inpfile *f);
i_INLINE_INS char *inpfile_sp(const struct inpfile *f);
i_INLINE_INS char *inpfile_lim(struct inpfile *f);
i_INLINE_INS uint64_t inpfile_ptr_to_offset(const struct inpfile *f, const char *p);
i_INLINE_INS struct RFstringx *inpfile_str(struct inpfile *f);
i_INLINE_INS struct RFstring *inpfile_name(struct inpfile *f);
i_INLINE_INS struct inpoffset *inpfile_offset(struct inpfile *f);
i_INLINE_INS bool inpfile_at_eof(struct inpfile *f);
i_INLINE_INS bool inpfile_has_synerr(struct inpfile *f);

