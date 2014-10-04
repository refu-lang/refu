#include <inplocation.h>

#include <inpstr.h>
#include <inpfile.h>

bool inplocation_init(struct inplocation *loc,
                      struct inpfile *f,
                      char *sp, char *ep)
{
    loc->start.p = sp;
    loc->end.p = ep;

    if (!inpstr_ptr_to_linecol(&f->str, loc->start.p,
                               &loc->start.line, &loc->start.col)) {
        return false;
    }

    if (ep) {
        if (!inpstr_ptr_to_linecol(&f->str, ep,
                                   &loc->end.line, &loc->end.col)) {
            return false;
        }
    }
    return true;
}
i_INLINE_INS void inplocation_init_marks(struct inplocation *loc,
                                          struct inplocation_mark *start,
                                          struct inplocation_mark *end);

void inplocation_set_start(struct inplocation *loc,
                           struct inplocation_mark *start)
{
    loc->start = *start;
}
void inplocation_set_end(struct inplocation *loc, struct inplocation_mark *end)
{
    loc->end = *end;
}

bool inplocation_from_file(struct inplocation *loc,
                           struct inpfile *f)
{
    struct inpstr *str = &f->str;

    if (!inpstr_ptr_to_linecol(str, inpstr_data(str),
                               &loc->start.line, &loc->start.col)) {
        ERROR("Could not create a location from a file");
        return false;
    }
    loc->end.line = loc->start.line;
    loc->end.col = loc->start.col;

    return true;
}


i_INLINE_INS bool inplocation_mark_equal(struct inplocation_mark *m1,
                                         struct inplocation_mark *m2);
i_INLINE_INS void inplocation_copy(struct inplocation *l1,
                                   struct inplocation *l2);
i_INLINE_INS bool inplocation_equal(struct inplocation *l1,
                                    struct inplocation *l2);
