#ifndef LFR_AST_LOCATION_H
#define LFR_AST_LOCATION_H

#include <Definitions/retcodes.h> //for bool
#include <Definitions/inline.h> //for bool

#include <parser/file.h>


struct parser_string;

struct ast_location {
    struct parser_file *file;
    unsigned int start_line;
    unsigned int start_col;
    unsigned int end_line;
    unsigned int end_col;
    char *sp;
    char *ep;
};

/* 2 macros for quick location initialization, mostly used in tests */
// initialize a location using only start and end line and columns
#define LOC_INIT(file_, sl_, sc_, el_, ec_)         \
    {.file = file_,                                 \
        .start_line = sl_,                          \
        .start_col = sc_,                           \
        .end_line = el_,                            \
        .end_col= ec_,                              \
        .sp = parser_file_line_p(file_, sl_) + sc_, \
        .ep = parser_file_line_p(file_, el_) + ec_}
// initialize a location using all attributes. Used if non-ascii chars in line
#define LOC_INIT_FULL(file_, sl_, sc_, el_, ec_, sp_, ep_)  \
    {.file = file_,                                         \
        .start_line = sl_,                                  \
        .start_col = sc_,                                   \
        .end_line = el_,                                    \
        .end_col= ec_,                                      \
        .sp = sp,                                           \
        .ep = ep_}

/**
 * Initialize an ast_location from a parser string.
 *
 * @param loc       The location to initialize
 * @param f         The parser file that contains the location
 * @param sp        Pointer at location start
 * @param eindex    Pointer at location end
 *
 * @return          True/false depending on success/failure
 */
bool ast_location_init(struct ast_location *loc,
                       struct parser_file *f,
                       char *sp, char *ep);

bool ast_location_set_end(struct ast_location *loc, char *end);

bool ast_location_from_file(struct ast_location *loc,
                            struct parser_file *f);

i_INLINE_DECL struct RFstring *ast_location_filename(struct ast_location *loc)
{
    return &loc->file->file_name;
}

i_INLINE_DECL void ast_location_copy(struct ast_location *l1,
                                     struct ast_location *l2)
{
    l1->file = l2->file;
    l1->start_line = l2->start_line;
    l1->start_col = l2->start_col;
    l1->end_line = l2->end_line;
    l1->end_col = l2->end_col;
    l1->sp = l2->sp;
    l1->ep = l2->ep;
}

/**
 * Comparison function assuming that the 2 locations are from the same file
 */
i_INLINE_DECL bool ast_location_equal(struct ast_location *l1,
                                      struct ast_location *l2)
{
    if (l1->start_line != l2->start_line) {
        return false;
    }
    if (l1->start_col != l2->start_col) {
        return false;
    }

    if (l1->end_line != l2->end_line) {
        return false;
    }
    if (l1->end_col != l2->end_col) {
        return false;
    }
    if (l1->sp != l2-> sp || l2->ep != l2->ep) {
        return false;
    }
    return true;
}


#define AST_LOCATION_FMT       \
    RF_STR_PF_FMT":%u:%u"
#define AST_LOCATION_ARG(loc_)                                          \
    RF_STR_PF_ARG(&(loc_)->file->file_name), (loc_)->start_line, (loc_)->start_col

#define AST_LOCATION_FMT2                       \
    RF_STR_PF_FMT":(%u:%u|%u:%u)"
#define AST_LOCATION_ARG2(loc_)                 \
    RF_STR_PF_ARG(&(loc_)->file->file_name),    \
        (loc_)->start_line, (loc_)->start_col,  \
        (loc_)->end_line, (loc_)->end_col

#endif
