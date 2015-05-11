#ifndef LFR_INPSTRING_H
#define LFR_INPSTRING_H

#include <RFstring.h>
#include <Definitions/inline.h> //for inline
#include <Utils/array.h>

struct inpstr {
    struct RFstringx str;
    uint32_t lines_num;
    uint32_t *lines;
};

/**
 * Initializes an input string from an RFstringx and some lines meta
 * information.
 *
 * @param s            The string to initialize
 * @param input_str    The RFstringx to initialize from.
 *                     We do a shallow copy of @c input_str
 *                     but also owns the memory of the string. Once intialized
 *                     it will manage the memory of input_str.
 * @param arr          The RFarray containing the lines information.
 *                     Indexing starts from 0 and contains the byte position
 *                     in @c input_str of each new line beginning. Its contents
 *                     will be copied inside the parser string. Can be freed
 *                     after string initialization
 * @param lines_num    The size of the @c arr in lines
 *
 * @return             True/false in case of success/failure to initialize
 */
bool inpstr_init(struct inpstr *s,
                 struct RFstringx *input_str,
                 struct RFarray *arr,
                 unsigned int lines_num);

void inpstr_deinit(struct inpstr *s);

/**
 * Obtain a line and column position from a byte pointer
 * of an input string
 *
 * @param s             The input string from which to obtain the position.
 * @param p             The byte pointer inside the string whose line and
 *                      column to retrieve
 * @param line[out]     Returns the line pointed to by the byte pointer
 * @param column[out]   Returns the column pointed to by the byte pointer
 *
 * @return              True if the byte pointer represents a valid position
 *                      and false if not
 */
bool inpstr_ptr_to_linecol(struct inpstr *s,
                           char *p, unsigned int *line,
                           unsigned int *col);

i_INLINE_DECL struct RFstringx *inpstr_str(struct inpstr *s)
{
    return &s->str;
}

i_INLINE_DECL char *inpstr_data(struct inpstr *s)
{
    return rf_string_data(&s->str);
}

i_INLINE_DECL char *inpstr_beg(const struct inpstr *s)
{
    return rf_string_data(&s->str) - s->str.bIndex;
}

i_INLINE_DECL uint32_t inpstr_len_from_beg(struct inpstr *s)
{
    return rf_string_length_bytes(&s->str) + s->str.bIndex;
}
#endif
