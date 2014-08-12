#ifndef LFR_PARSER_FILE_H
#define LFR_PARSER_FILE_H

#include <RFstring.h>
#include <RFintrusive_list.h>

#include <parser/string.h>
#include <parser/offset.h>

struct ast_node;

struct parser_file {
    struct RFstring file_name;
    struct parser_string pstr;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
    char *bp;
    struct parser_offset offset;
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
#endif
