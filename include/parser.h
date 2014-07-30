#include <RFintrusive_list.h>
#include <RFstring.h>

struct parser_file {
    struct RFstring file_name;
    struct RFstringx buffer;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
    char *bp;
};

struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
};


// Gets a parser's context stringx buffer
#define PARSER_CBUFFP(p) &(p)->current_file->buffer

struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                        const struct RFstring *name);


static inline void parser_accept_ws(struct parser_ctx *parser)
{
    /* rf_stringx_move_afterv(&parser->current_file->file_name, */
    /*                        NULL, //don't keep what's passed */
    /*                        0, //no specific options */
    /*                        3, */
    /*                        " ", "\t", "\n"); */

    /* char *sp = rf_string_data(&parser->current_file->buffer); */
    /* unsigned int length = rf_string_length(&parser->current_file->buffer); */
    /* char *end = sp + length; */
    /* while (sp <= end && sp == ' ' || sp == '\t' || sp == '\n') { */
    /*     sp ++; */
    /* } */

    //TODO: implement this in Refulib
    unsigned int bytes_moved;
    unsigned int chars_moved;
    rf_stringx_move_after_any(PARSER_CBUFF(parser),
                              &bytes_moved,
                              &chars_moved,
                              3
                              ' ', '\t', '\n');

}
