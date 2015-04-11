#include <analyzer/type_set.h>
#include <types/type.h>
#include <types/type_comparisons.h>


i_INLINE_INS const void *type_objset_key(const struct type *t);

size_t type_objset_hashfn(const struct type *t)
{
    return type_get_uid(t);
}

bool type_objset_eqfn(const struct type *t1,
                      const struct type *t2)
{
    return type_compare(t1, t2, TYPECMP_IDENTICAL);
}
