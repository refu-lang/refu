#include <analyzer/type_set.h>
#include <types/type.h>
#include <types/type_comparisons.h>


i_INLINE_INS const void *type_objset_key(const struct type *t);

size_t type_objset_hashfn(const struct type *t)
{
    return type_get_uid(t, true);
}

bool type_objset_eqfn(const struct type *t1,
                      const struct type *t2)
{
    return type_compare(t1, t2, TYPECMP_IDENTICAL);
}

struct type *type_objset_has_convertable(const struct rf_objset_type *set, const struct type *type)
{
    struct rf_objset_iter it1;
    struct type *t1;
    
    rf_objset_foreach(set, &it1, t1) {
        if (type_compare(type, t1, TYPECMP_IMPLICIT_CONVERSION)) {
            return t1;
        }
    }
    return NULL;
}

static bool uid_cmp_fn(struct type *t, size_t *uid)
{
    return type_get_uid(t, true) == *uid;
}

struct type *type_objset_has_uid(const struct rf_objset_type *set, size_t uid)
{
    return htable_get(&set->raw.ht,
                      uid,
                      (bool (*)(const void *, void *))uid_cmp_fn,
                      &uid);
}

struct type *type_objset_has_string(const struct rf_objset_type *set, const struct RFstring *desc)
{
    return type_objset_has_uid(set, rf_hash_str_stable(desc, 0));
}

void type_objset_destroy(struct rf_objset_type *set,
                         struct rf_fixed_memorypool *types_pool)
{
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(set, &it, t) {
        type_free(t, types_pool);
    }
    rf_objset_clear(set);
    free(set);
}
