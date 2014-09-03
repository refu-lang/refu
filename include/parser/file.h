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

/**
 * Indicates eof has been reached for this file
 */
bool parser_file_eof(struct parser_file *f);
void parser_file_move(struct parser_file *f,
                      unsigned int bytes,
                      unsigned int chars);
void parser_file_move_to_offset(struct parser_file *f,
                                struct parser_offset *off);


/**
 * Convenience macro to call a parser related statement and if it results
 * into end of file perform some action
 */
#define PARSER_CHECK_EOF(STMT_, FILE_, ACTION_) \
    do {                                        \
        STMT_;                                  \
        if (parser_file_eof(FILE_)) {           \
            ACTION_;                            \
        }                                       \
    } while(0)

/**
 * Accepts an arbitrary number of whitespaces and moves the file pointer
 * by that amount
 *
 * @param f         The parser file to work with
 */
void parser_file_acc_ws(struct parser_file *f);
/**
 * Accepts a specific ASCII string and moves the file pointer right after it
 * The string does not need to be space separated. "foo" will match
 * both "foo" and "foosomethingelse". If moving the pointer would take it off
 * the file limits then it does not move afterwards.
 *
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

i_INLINE_DECL struct parser_offset *parser_file_offset(struct parser_file *f)
{
    return &f->offset;
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



/**
 * A macro to append a syntax error to the parser context
 *
 * @param file_           The file which the error concerns
 * @param locmarkoff_     An optional offset at which to place
 *                        the location marker compared to the
 *                        current file position. Can be of use when
 *                        we want to show that the problem lies with
 *                        the next character
 */
#define parser_file_synerr(file_, locmarkoff_, ...) \
    do {                                            \
        struct ast_location i_loc_;                 \
        ast_location_from_file(&i_loc_, (file_));   \
        i_loc_.start_col += locmarkoff_;            \
        (file_)->info->syntax_error = true;         \
        i_info_ctx_add_msg((file_)->info,           \
                           MESSAGE_SYNTAX_ERROR,    \
                           &i_loc_,                 \
                           __VA_ARGS__);            \
    } while(0)

#endif
