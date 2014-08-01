#ifndef RF_PARSER_H
#define RF_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <parser/offset.h>

struct parser_file {
    struct RFstring file_name;
    struct RFstringx buffer;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
    char *bp;
    struct parser_offset offset;
};

struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
};

// Gets a parser's context current stringx buffer
#define PARSER_CBUFF(p) &(p)->current_file->buffer
// Gets a parser's current file offset
#define PARSER_COFF(p) &(p)->current_file->offset

struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                        const struct RFstring *name);

i_INLINE_DECL void parser_move_to_offset(struct parser_ctx *parser,
                                         struct parser_offset *off)
{
    rf_stringx_move_to_index(PARSER_CBUFF(parser), off->bytes_moved);
    parser_offset_copy(PARSER_COFF(parser), off);
}

i_INLINE_DECL void parser_move(struct parser_ctx *parser,
                               unsigned int bytes,
                               unsigned int chars)
{
    parser_offset *off = PARSER_COFF(parser);
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    
    off->bytes_moved += bytes;
    off->chars_moved += chars;
    off->lines_moved += rf_string_count(PARSER_CBUFF(parser), &nl, 0);

    rf_stringx_move_to_index(PARSER_CBUFF(parser), bytes);
}


static inline void parser_accept_ws(struct parser_ctx *parser)
{
    
    static const struct RFstring wsp = RF_STRING_STATIC_INIT(" \t\n\r");
    struct parser_offset mov;
    mov.chars_moved += rf_stringx_skip_chars(PARSER_CBUFF(parser),
                                             &wsp,
                                             &mov.bytes_moved,
                                             &mov.lines_moved);
    parser_offset_add(PARSER_COFF(parser), &mov);
}

static inline bool parser_accept_string_ascii(struct parser_ctx *parser,
                                              const struct RFstring *str)
{
    struct RFstringx *buff = PARSER_CBUFF(parser);
    if(!rf_string_begins_with(buff, str, 0)) {
        return false;
    }

    parser_move(parser, str->length, str->length);
    return true;
}

#endif
