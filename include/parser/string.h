#ifndef LFR_PARSER_STRING_H
#define LFR_PARSER_STRING_H

#include <RFstring.h>
#include <Definitions/inline.h> //for inline
#include <Utils/array.h>

struct parser_string {
    struct RFstringx str;
    uint32_t lines_num;
    uint32_t *lines;
};

/**
 * Initializes a parser string from an RFstringx and some lines meta
 * information.
 *
 * @param s            The string to initialize
 * @param input_str    The RFstringx to initialize from
 *                     Parser string does a shallow copy of @c input_str
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
bool parser_string_init(struct parser_string *s,
                        struct RFstringx *input_str,
                        struct RFarray *arr,
                        unsigned int lines_num);

void parser_string_deinit(struct parser_string *s);


i_INLINE_DECL char *parser_string_data(struct parser_string *s)
{
    return rf_string_data(&s->str);
}



#endif
