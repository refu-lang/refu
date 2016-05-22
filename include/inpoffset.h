#ifndef LFR_INPOFFSET_H
#define LFR_INPOFFSET_H

#include <rfbase/defs/inline.h>

struct inpoffset {
    unsigned int bytes_moved;
    unsigned int chars_moved;
    unsigned int lines_moved;
};


#define INPOFFSET_STATIC_INIT() {0, 0, 0}
i_INLINE_DECL void inpoffset_init(struct inpoffset *off)
{
    off->bytes_moved = 0;
    off->chars_moved = 0;
    off->lines_moved = 0;
}

i_INLINE_DECL void inpoffset_copy(struct inpoffset *dst,
                                  struct inpoffset *src)
{
    dst->bytes_moved = src->bytes_moved;
    dst->chars_moved = src->chars_moved;
    dst->lines_moved = src->lines_moved;
}

i_INLINE_DECL void inpoffset_add(struct inpoffset *o1,
                                 struct inpoffset *o2)
{
    o1->bytes_moved += o2->bytes_moved;
    o1->chars_moved += o2->chars_moved;
    o1->lines_moved += o2->lines_moved;
}

i_INLINE_DECL void inpoffset_sub(struct inpoffset *o1,
                                 struct inpoffset *o2)
{
    o1->bytes_moved -= o2->bytes_moved;
    o1->chars_moved -= o2->chars_moved;
    o1->lines_moved -= o2->lines_moved;
}
#endif
