#include <analyzer/type_set.h>
#include <types/type.h>
#include <types/type_comparisons.h>
#include <types/type_operators.h>


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
    return type_get_uid(t) == *uid;
}

struct type *type_objset_has_uid(const struct rf_objset_type *set, size_t uid)
{
    return htable_get(
        &set->raw.ht,
        uid,
        (bool (*)(const void *, void *))uid_cmp_fn,
        &uid
    );
}

struct type *type_objset_has_string(const struct rf_objset_type *set, const struct RFstring *desc)
{
    return type_objset_has_uid(set, rf_hash_str_stable(desc, 0));
}

void type_objset_destroy(
    struct rf_objset_type *set,
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

static void array_types_append(struct arr_types *arr, struct type *t)
{
    struct type **it_t;
    darray_foreach(it_t, *arr) {
        if (t == *it_t) {
            return;
        }
    }
    darray_append(*arr, t);
}

bool typeset_to_ordered_array(struct rf_objset_type *set, struct arr_types *arr)
{
    struct rf_objset_iter it;
    struct type *t;
    struct rf_objset_iter it2;
    struct type *t2;
    struct arr_types depending_types;
    darray_init(*arr);
    rf_objset_foreach(set, &it, t) {
        darray_init(depending_types);
        rf_objset_foreach(set, &it2, t2) {
            if (t2 != t) {
                if (type_is_childof(t2, t) != -1) {
                    darray_append(depending_types, t2);
                }
            }
        }
        if (darray_size(depending_types) == 0) {
            array_types_append(arr, t);
        } else {
            struct type **it_t;
            darray_foreach(it_t, depending_types) {
                array_types_append(arr, *it_t);
            }
            array_types_append(arr, t);
        }
        darray_free(depending_types);
    }
    return true;
}

#ifdef RF_OPTION_DEBUG
void type_objset_print(struct rf_objset_type *set)
{
    struct rf_objset_iter it;
    struct type *t;
    printf("Printing members of a typeset\n\n");
    RFS_PUSH();
    rf_objset_foreach(set, &it, t) {
        printf(RFS_PF"\n", RFS_PA(type_str_or_die(t, TSTR_DEFAULT)));
    }
    RFS_POP();
    printf("\n\n");
    fflush(stdout);
}
#endif
