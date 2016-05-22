#ifndef LFR_INPLOCATION_H
#define LFR_INPLOCATION_H

#include <rfbase/defs/retcodes.h>
#include <rfbase/defs/inline.h>

#include <inpfile.h>

struct inplocation_mark {
    unsigned int line;
    unsigned int col;
    char *p;
};

i_INLINE_DECL bool inplocation_mark_equal(const struct inplocation_mark *m1,
                                          const struct inplocation_mark *m2)
{
    if (m1->line != m2->line) {
        return false;
    }
    if (m1->col != m2->col) {
        return false;
    }
    if (m1->p != m2->p) {
        return false;
    }
    return true;
}

i_INLINE_DECL bool inplocation_mark_empty(const struct inplocation_mark *m)
{
    return m->p == 0;
}

struct inplocation {
    struct inplocation_mark start;
    struct inplocation_mark end;
};


#define LOCMARK_RESET(mark_)                    \
    do {                                        \
        (mark_)->line = 0;                      \
        (mark_)->col = 0;                       \
        (mark_)->p = 0;                         \
    } while(0)

#define LOCMARK_INIT_ZERO()                     \
    {                                           \
        .line = 0,                              \
        .col = 0,                               \
        .p = 0                                  \
    }

#define LOCMARK_INIT(file_, line_, col_)          \
    {                                             \
        .line = line_,                            \
        .col = col_,                              \
        .p = inpfile_line_p(file_, line_) + col_  \
    }

/* 2 macros for quick location initialization, mostly used in tests */
// initialize a location using only start and end line and columns
#define LOC_INIT(file_, sl_, sc_, el_, ec_)       \
    {                                             \
        .start = LOCMARK_INIT(file_, sl_, sc_),   \
        .end = LOCMARK_INIT(file_, el_, ec_)      \
    }

// initialize a location using all attributes. Used if non-ascii chars in line
#define LOC_INIT_FULL(sl_, sc_, el_, ec_, sp_, ep_)         \
    {                                                       \
        .start = {                                          \
            .line = sl_,                                    \
            .col = sc_,                                     \
            .p = sp_                                        \
        },                                                  \
                                                            \
        .end = {                                            \
            .line = el_,                                    \
            .col = ec_,                                     \
            .p = ep_                                        \
        }                                                   \
    }

/**
 * Initialize a location from pointers to an input string
 *
 * @param loc       The location to initialize
 * @param f         The file the location refers to
 * @param sp        Pointer at location start
 * @param ep        Pointer at location end
 *
 * @return          True/false depending on success/failure
 */
bool inplocation_init(struct inplocation *loc,
                      struct inpfile *f,
                      char *sp, char *ep);
/**
 * Initialize a location from 2 location marks
 *
 * @param loc       The location to initialize
 * @param start     The starting location mark
 * @param end       The ending location mark. Can be NULL and be
 *                  filled in after initialization
 */
i_INLINE_DECL void inplocation_init_marks(struct inplocation *loc,
                                          const struct inplocation_mark *start,
                                          const struct inplocation_mark *end)
{
    loc->start = *start;
    if (end) {
        loc->end = *end;
    }
}

void inplocation_set_start(struct inplocation *loc,
                           const struct inplocation_mark *start);
void inplocation_set_end(struct inplocation *loc,
                         const struct inplocation_mark *end);

bool inplocation_from_file_at_point(struct inplocation *loc,
                                    struct inpfile *f,
                                    char *p);
bool inplocation_from_file(struct inplocation *loc,
                           struct inpfile *f);

i_INLINE_DECL void inplocation_copy(struct inplocation *l1,
                                    const struct inplocation *l2)
{
    l1->start = l2->start;
    l1->end = l2->end;
}

i_INLINE_DECL bool inplocation_equal(const struct inplocation *l1,
                                     const struct inplocation *l2)
{
    if (!inplocation_mark_equal(&l1->start, &l2->start)) {
        return false;
    }
    if (!inplocation_mark_equal(&l1->end, &l2->end)) {
        return false;
    }
    return true;
}


#define INPLOCATION_FMT                         \
    RFS_PF":%u:%u"
#define INPLOCATION_ARG(file_, loc_)                                    \
    RFS_PA(&(file_)->file_name), (loc_)->start.line, (loc_)->start.col

#define INPLOCATION_FMT2                        \
    RFS_PF":(%u:%u|%u:%u)"
#define INPLOCATION_ARG2(file_, loc_)           \
    RFS_PA(&(file_)->file_name),                \
        (loc_)->start.line, (loc_)->start.col,  \
        (loc_)->end.line, (loc_)->end.col

#define INPLOCMARKS_FMT                         \
    RFS_PF":(%u:%u|%u:%u)"
#define INPLOCMARKS_ARG(file_, start_, end_)    \
    RFS_PA(&(file_)->file_name),                \
        (start_)->line, (start_)->col,          \
        (end_)->line, (end_)->col


#endif
