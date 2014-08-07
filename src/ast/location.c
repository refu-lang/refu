#include <ast/location.h>

bool ast_location_init(struct ast_location *loc,
                       struct parser_file *f,
                       char *sp, char *ep)
{
    loc->file = f;
    loc->sp = sp;
    loc->ep = ep;

    if (!parser_string_ptr_to_linecol(&f->pstr, sp, &start_line, &start_col)) {
        return false;
    }

    if (!parser_string_ptr_to_linecol(&f->pstr, ep, &end_line, &end_col)) {
        return false;
    }

    return true;
}

i_INLINE_INS void ast_location_copy(struct ast_location *l1,
                                    struct ast_location *l2);
