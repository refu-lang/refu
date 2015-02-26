#include <ir/rir_types_list.h>

#include <stdio.h>
#include <String/rf_str_core.h>

#include <types/type_elementary.h>
#include <types/type.h>
#include <ir/rir_type.h>

#define ELEMENTARY_RIR_TYPE_INIT(category_)        \
    [ELEMENTARY_TYPE_ ## category_] = { .category = ELEMENTARY_RIR_TYPE_ ## category_, .indexed = true, .name = NULL }
struct rir_type elementary_rir_types_[] = {
    ELEMENTARY_RIR_TYPE_INIT(INT),
    ELEMENTARY_RIR_TYPE_INIT(UINT),
    ELEMENTARY_RIR_TYPE_INIT(INT_8),
    ELEMENTARY_RIR_TYPE_INIT(UINT_8),
    ELEMENTARY_RIR_TYPE_INIT(INT_16),
    ELEMENTARY_RIR_TYPE_INIT(UINT_16),
    ELEMENTARY_RIR_TYPE_INIT(INT_32),
    ELEMENTARY_RIR_TYPE_INIT(UINT_32),
    ELEMENTARY_RIR_TYPE_INIT(INT_64),
    ELEMENTARY_RIR_TYPE_INIT(UINT_64),
    ELEMENTARY_RIR_TYPE_INIT(FLOAT_32),
    ELEMENTARY_RIR_TYPE_INIT(FLOAT_64),
    ELEMENTARY_RIR_TYPE_INIT(STRING),
    ELEMENTARY_RIR_TYPE_INIT(BOOL),
    ELEMENTARY_RIR_TYPE_INIT(NIL)
};
#undef ELEMENTARY_RIR_TYPE_INIT

void rir_types_list_init(struct rir_types_list *l)
{
    rf_ilist_head_init(&l->lh);
}

void rir_types_list_deinit(struct rir_types_list *l)
{
    struct rir_type *t;
    struct rir_type *tmp;

    rf_ilist_for_each_safe(&l->lh, t, tmp, ln) {
        rir_type_destroy(t);
    }
}

struct rir_type *rir_types_list_get_defined(struct rir_types_list *list,
                                            const struct RFstring *name)
{
    struct rir_type *t;
    rf_ilist_for_each(&list->lh, t, ln) {
        if (t->category == COMPOSITE_RIR_DEFINED && rf_string_equal(t->name, name)) {
            return t;
        }
    }
    return NULL;
}

struct rir_type *rir_types_list_get_type(struct rir_types_list *list,
                                         struct type *type,
                                         const struct RFstring *name)
{
    struct rir_type *iter_rir_type;

    if (!name && type->category == TYPE_CATEGORY_ELEMENTARY) {
        return &elementary_rir_types_[type_elementary(type)];
    }
    rf_ilist_for_each(&list->lh, iter_rir_type, ln) {
        if (rir_type_equals_type(iter_rir_type, type, name)) {
                return iter_rir_type;
        }
    }
    return NULL;
}

// very very temporary macro to allow visualization of rir type creation. Will go away
 /* #define TEMP_RIR_DEBUG 1 */
bool rir_types_list_populate(struct rir_types_list *rir_types, struct RFilist_head *composite_types)
{
    struct type *t;
    struct rir_type *iter_rir_type;
    struct rir_type *created_rir_type;
    bool found;
    rf_ilist_for_each(composite_types, t, lh) {
#if TEMP_RIR_DEBUG
        RFS_buffer_push();
        printf("iterating type: "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str(t, true)));
        fflush(stdout);
#endif
        // first of all see if this composite type already has an equivalent rir type
        found = false;
        rf_ilist_for_each(&rir_types->lh, iter_rir_type, ln) {
            if (rir_type_equals_type(iter_rir_type, t, NULL)) {
                found = true;
            }
        }
        // if it does don't bother with it anymore
        if (found) {
            continue;
        }

        created_rir_type = rir_type_create(t, NULL, NULL);
#if TEMP_RIR_DEBUG
        printf("created rir type: "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(rir_type_str(created_rir_type)));
        fflush(stdout);
        RFS_buffer_pop();
#endif
        if (!created_rir_type) {
            RF_ERROR("Failed to create a rir type during transition Refu IR.");
            return false;
        }
        rf_ilist_add_tail(&rir_types->lh, &created_rir_type->ln);
        created_rir_type->indexed = true;
    }

    return true;
}
