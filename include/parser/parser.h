#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <parser/offset.h>
#include <parser/file.h>

struct info_ctx;
struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
    struct info_ctx *info;
};


struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                        const struct RFstring *name);

i_INLINE_DECL struct RFstringx *parser_curr_str(struct parser_ctx *p)
{
    return &p->current_file->pstr.str;
}

i_INLINE_DECL struct parser_string *parser_curr_pstr(struct parser_ctx *p)
{
    return &p->current_file->pstr;
}

i_INLINE_DECL char *parser_curr_sp(struct parser_ctx *p)
{
    return rf_string_data(&p->current_file->pstr.str);
}

i_INLINE_DECL struct parser_offset *parser_curr_off(struct parser_ctx *p)
{
    return &p->current_file->offset;
}

i_INLINE_DECL void parser_move_to_offset(struct parser_ctx *parser,
                                         struct parser_offset *off)
{
    rf_stringx_move_to_index(parser_curr_str(parser), off->bytes_moved);
    parser_offset_copy(parser_curr_off(parser), off);
}

i_INLINE_DECL void parser_move(struct parser_ctx *parser,
                               unsigned int bytes,
                               unsigned int chars)
{
    struct parser_offset *off = parser_curr_off(parser);
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    
    off->bytes_moved += bytes;
    off->chars_moved += chars;
    off->lines_moved += rf_string_count(parser_curr_str(parser), &nl, 0);

    rf_stringx_move_to_index(parser_curr_str(parser), bytes);
}


i_INLINE_DECL void parser_accept_ws(struct parser_ctx *parser)
{
    
    static const struct RFstring wsp = RF_STRING_STATIC_INIT(" \t\n\r");
    struct parser_offset mov;
    mov.chars_moved += rf_stringx_skip_chars(parser_curr_str(parser),
                                             &wsp,
                                             &mov.bytes_moved,
                                             &mov.lines_moved);
    parser_offset_add(parser_curr_off(parser), &mov);
}

i_INLINE_DECL bool parser_accept_string_ascii(struct parser_ctx *parser,
                                              const struct RFstring *str)
{
    struct RFstringx *buff = parser_curr_str(parser);
    if(!rf_string_begins_with(buff, str, 0)) {
        return false;
    }

    parser_move(parser, str->length, str->length);
    return true;
}

#endif
