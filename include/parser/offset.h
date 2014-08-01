#ifndef RF_PARSER_OFFSET_H
#define RF_PARSER_OFFSET_H

struct parser_offset {
    unsigned int bytes_moved;
    unsigned int chars_moved;
    unsigned int lines_moved;
};

#define PARSER_OFFSET_INIT() {0, 0, 0}

i_INLINE_DECL void parser_offset_copy(struct parser_offset *dst,
                                      struct parser_offset *src)
{
    dst->bytes_moved = src->bytes_moved;
    dst->chars_moved = src->chars_moved;
    dst->lines_moved = src->lines_moved;
}

i_INLINE_DECL void parser_offset_add(struct parser_offset *o1,
                                     struct parser_offset *o2)
{
    o1->bytes_moved += o2->bytes_moved;
    o1->chars_moved += o2->chars_moved;
    o1->lines_moved += o2->lines_moved;
}

i_INLINE_DECL void parser_offset_sub(struct parser_offset *o1,
                                     struct parser_offset *o2)
{
    o1->bytes_moved -= o2->bytes_moved;
    o1->chars_moved -= o2->chars_moved;
    o1->lines_moved -= o2->lines_moved;
}
#endif
