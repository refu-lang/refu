#include <analyzer/type_set.h>
#include <types/type.h>
#include <types/type_comparisons.h>
#include <ir/rir_type.h>


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

bool type_objset_has_convertable(const struct rf_objset_type *set, const struct type *type)
{
    struct rf_objset_iter it1;
    struct type *t1;
    
    rf_objset_foreach(set, &it1, t1) {
        if (type_compare(type, t1, TYPECMP_IMPLICIT_CONVERSION)) {
            return true;
        }
    }
    return false;
}


struct type *type_objset_get_rir_type(const struct rf_objset_type *set,
                                      const struct rir_type *type)
{
    struct rf_objset_iter it1;
    struct type *t1;
    
    rf_objset_foreach(set, &it1, t1) {
        if (rir_type_equals_type(type, t1, NULL)) {
            return t1;
        }
    }
    return NULL;
}
