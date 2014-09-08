#ifndef LFR_INPLOCATION_H
#define LFR_INPLOCATION_H

#include <Definitions/retcodes.h> //for bool
#include <Definitions/inline.h>

#include <inpfile.h>

struct inplocation_mark {
    unsigned int line;
    unsigned int col;
    char *p;
};

i_INLINE_DECL bool inplocation_mark_equal(struct inplocation_mark *m1,
                                          struct inplocation_mark *m2)
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


struct inplocation {
    struct inplocation_mark start;
    struct inplocation_mark end;
};

/* 2 macros for quick location initialization, mostly used in tests */
// initialize a location using only start and end line and columns
#define LOC_INIT(file_, sl_, sc_, el_, ec_)       \
    {                                             \
        .start = {                                \
            .line = sl_,                          \
            .col = sc_,                           \
            .p = inpfile_line_p(file_, sl_) + sc_ \
        },                                        \
                                                  \
        .end = {                                  \
            .line = el_,                          \
            .col = ec_,                           \
            .p = inpfile_line_p(file_, el_) + ec_ \
        }                                         \
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
                                          struct inplocation_mark *start,
                                          struct inplocation_mark *end)
{
    loc->start = *start;
    if (end) {
        loc->end = *end;
    }
}

void inplocation_set_end(struct inplocation *loc, struct inplocation_mark *end);

bool inplocation_from_file(struct inplocation *loc,
                           struct inpfile *f);

i_INLINE_DECL void inplocation_copy(struct inplocation *l1,
                                    struct inplocation *l2)
{
    l1->start = l2->start;
    l1->end = l2->end;
}

i_INLINE_DECL bool inplocation_equal(struct inplocation *l1,
                                     struct inplocation *l2)
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
    RF_STR_PF_FMT":%u:%u"
#define INPLOCATION_ARG(file_, loc_)                                    \
    RF_STR_PF_ARG(&(file_)->file_name), (loc_)->start.line, (loc_)->start.col

#define INPLOCATION_FMT2                        \
    RF_STR_PF_FMT":(%u:%u|%u:%u)"
#define INPLOCATION_ARG2(file_, loc_)           \
    RF_STR_PF_ARG(&(file_)->file_name),         \
        (loc_)->start.line, (loc_)->start.col,  \
        (loc_)->end.line, (loc_)->end.col

#endif
