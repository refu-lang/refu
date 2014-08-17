#ifndef LFR_PARSER_FILE_H
#define LFR_PARSER_FILE_H

#include <RFstring.h>
#include <RFintrusive_list.h>

#include <parser/string.h>
#include <parser/offset.h>
#include <info/info.h>

struct ast_node;

struct parser_file {
    struct RFstring file_name;
    struct parser_string pstr;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
    char *bp;
    struct parser_offset offset;
    struct info_ctx *info;
    struct ast_node *root;
};

struct parser_file *parser_file_new(const struct RFstring *name);
void parser_file_deinit(struct parser_file *f);

bool parser_file_eof(struct parser_file *f);
void parser_file_move(struct parser_file *f,
                      unsigned int bytes,
                      unsigned int chars);
void parser_file_move_to_offset(struct parser_file *f,
                                struct parser_offset *off);

/**
 * Accepts an arbitrary number of whitespaces and moves the file pointer
 * by that amount
 *
 * @param f         The parser file to work with
 */
void parser_file_acc_ws(struct parser_file *f);
/**
 * Accepts a specific ASCII string and moves the file pointer after it
 *
 * @param f         The parser file to work with
 * @param str       The string to accept
 *
 * @return          True if the string was found and false otherwise
 */
bool parser_file_acc_string_ascii(struct parser_file *f,
                                  const struct RFstring *str);

/**
 * Looks ahead from the current position for a specific string
 *
 * @param f         The parser file to work with
 * @param str       The string to look for
 * @param end       If not NULL a pointer to @c str, to limit the search
 * @return          Pointer to the position the string is found or NULL if not
 *                  found
 */
char *parser_file_lookfor(struct parser_file *f,
                          const struct RFstring *str,
                          char *end);

/**
 * Gets the current string pointer in the file
 *
 * @param f         The parser file to work with
 *
 * @return          A char* pointing to the current position in the file's str
 */
i_INLINE_DECL char *parser_file_sp(struct parser_file *f)
{
    return rf_string_data(&f->pstr.str);
}

/**
 * Gets the string of the parser file
 *
 * @param f         The parser file to work with
 *
 * @return          The stringx of this parser file
 */
i_INLINE_DECL struct RFstringx *parser_file_str(struct parser_file *f)
{
    return &f->pstr.str;
}

/**
 * Returns if there has been a syntax error during parsing the file
 */
i_INLINE_DECL bool parser_file_has_synerr(struct parser_file *f)
{
    //or:
    // return f->info->syntax_error;
    return info_ctx_has(f->info, MESSAGE_SYNTAX_ERROR);
}

/**
 * Gets a line from a parser file and a char pointer.
 *
 * @param f          The parser file to work with
 * @param p          The position pointer whose line to get
 * @param str[out]   Initializes a string with the data of the line we
 *                   need to return. Points to the file, no need to free
 * @return           True for success and false for failure.
 */
bool parser_file_line(struct parser_file *f,
                      uint32_t line,
                      struct RFstring *str);



#define parser_file_synerr(file_, ...)            \
    do {                                          \
        struct ast_location i_loc_;               \
        ast_location_from_file(&i_loc_, (file_)); \
        (file_)->info->syntax_error = true;       \
        i_info_ctx_add_msg((file_)->info,         \
                           MESSAGE_SYNTAX_ERROR,  \
                           &i_loc_,               \
                           __VA_ARGS__);          \
    } while(0)

#endif
