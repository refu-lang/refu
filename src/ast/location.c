#include <ast/location.h>

#include <parser/string.h>
#include <parser/file.h>

bool ast_location_init(struct ast_location *loc,
                       struct parser_file *f,
                       char *sp, char *ep)
{
    loc->file = f;
    loc->sp = sp;
    loc->ep = ep;

    if (!parser_string_ptr_to_linecol(&f->pstr, sp,
                                      &loc->start_line, &loc->start_col)) {
        return false;
    }

    if (!parser_string_ptr_to_linecol(&f->pstr, ep,
                                      &loc->end_line, &loc->end_col)) {
        return false;
    }

    return true;
}

bool ast_location_from_file(struct ast_location *loc,
                            struct parser_file *f)
{
    struct parser_offset *off = &f->offset;
    struct parser_string *pstr = &f->pstr;
    loc->file = f;

    if (!parser_string_ptr_to_linecol(pstr, parser_string_beg(pstr),
                                      &loc->start_line, &loc->start_col)) {
        ERROR("Could not create a location from a file");
        return false;
    }
    loc->end_line = loc->start_line;
    loc->end_col = loc->start_col;

    return true;
}

i_INLINE_INS void ast_location_copy(struct ast_location *l1,
                                    struct ast_location *l2);
