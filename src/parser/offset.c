#include <parser/offset.h>

i_INLINE_INS void parser_offset_init(struct parser_offset *off);
i_INLINE_INS void parser_offset_copy(struct parser_offset *dst,
                                     struct parser_offset *src);
i_INLINE_INS void parser_offset_add(struct parser_offset *o1,
                                    struct parser_offset *o2);
i_INLINE_INS void parser_offset_sub(struct parser_offset *o1,
                                    struct parser_offset *o2);


