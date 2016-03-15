#ifndef LFR_INPFILE_H
#define LFR_INPFILE_H

#include <rflib/datastructs/intrusive_list.h>

#include <info/info.h>
#include <inpstr.h>
#include <inpoffset.h>

struct inpfile {
    struct RFstring file_name;
    struct inpstr str;
    struct info_ctx *info;
    struct inpoffset offset;
    struct ast_node *root;
};


struct inpfile *inpfile_create(const struct RFstring *name);
struct inpfile *inpfile_create_from_string(const struct RFstring *name,
                                           const struct RFstring *contents);

void inpfile_deinit(struct inpfile* f);
void inpfile_destroy(struct inpfile *f);

/**
 * Accepts an arbitrary number of whitespaces and moves the file's
 * string pointer by that amount
 *
 * @param f         The file to work with
 */
void inpfile_acc_ws(struct inpfile *f);
/**
 * Move the internal RFstringx pointer of an inpfile
 *
 * @param f          The file to work with
 * @param bytes      The bytes to move
 * @param chars      The chars to move. TODO: rethink this. makes no sense
 */
void inpfile_move(struct inpfile *f,
                  unsigned int bytes,
                  unsigned int chars);


/**
 * Gets a specific line's string from an input file
 *
 * @param f          The input file to work with
 * @param p          The position pointer whose line to get
 * @param str[out]   Initializes a string with the data of the line we
 *                   need to return. Points to the file, no need to free
 * @return           True for success and false for failure.
 */
bool inpfile_line(struct inpfile *f,
                  uint32_t line,
                  struct RFstring *str);
/**
 * Gets the string pointer to the beginning of a specific line
 *
 * @param f        The input file to work with
 * @param line     The line whose beginning we need
 * @return         A char* pointing to the beginning of @c line.
 *                 If line does not exist return NULL.
 */
char *inpfile_line_p(struct inpfile *f, unsigned int line);

/**
 * Gets the RFstring of the input file at its current state
 *
 * @param f         The input file to work with
 *
 * @return          The stringx of this parser file
 */
i_INLINE_DECL struct RFstringx *inpfile_str(struct inpfile *f)
{
    return &f->str.str;
}

/**
 * Gets the current string pointer in the file
 *
 * @param f         The input file to work with
 *
 * @return          A char* pointing to the current position in the file's str
 */
i_INLINE_DECL char *inpfile_p(struct inpfile *f)
{
    return inpstr_data(&f->str);
}

/**
 * Gets a c string pointer to the beginning of the file
 *
 * @param f         The input file to work with
 *
 * @return          A char* pointing to the start position of the file's str
 */
i_INLINE_DECL char *inpfile_sp(const struct inpfile *f)
{
    return inpstr_beg(&f->str);
}

/**
 * Gets a c string pointer to the end of the file, acting as a limit for search
 *
 * @param f         The input file to work with
 *
 * @return          A char* pointing to the end position of the file's str
 */
i_INLINE_DECL char *inpfile_lim(struct inpfile *f)
{
    return inpfile_sp(f) + rf_string_length_bytes(inpfile_str(f)) - 1;
}

/**
 * Obtain a byte offset into the file given a byte pointer
 *
 * @param f             The input file for which to get the offset
 * @param p             The pointer whose offset into the string to get.
 *                      If this is invalid then behaviour is undefined.
 */
i_INLINE_DECL uint64_t inpfile_ptr_to_offset(const struct inpfile *f, const char *p)
{
    return p - inpfile_sp(f);
}

/**
 * Gets the name of the file
 */
i_INLINE_DECL struct RFstring *inpfile_name(struct inpfile *f)
{
    return &f->file_name;
}

/**
 * Gets the byte/char/line offset to the file
 */
i_INLINE_DECL struct inpoffset *inpfile_offset(struct inpfile *f)
{
    return &f->offset;
}

/**
 * Check if the file's string internal pointer is at the end of the file
 */
i_INLINE_DECL bool inpfile_at_eof(struct inpfile *f)
{
    return rf_string_length_bytes(&f->str.str) == 0;
}

/**
 * Returns if there has been a syntax error during parsing the file
 */
i_INLINE_DECL bool inpfile_has_synerr(struct inpfile *f)
{
    //or:
    // return f->info->syntax_error;
    return info_ctx_has(f->info, MESSAGE_SYNTAX_ERROR);
}

#endif
