#include <ir/rir_types_list.h>

#include <stdio.h>
#include <String/rf_str_core.h>

#include <analyzer/type_set.h>
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

static void rir_type_purge_from_subtypes(struct RFilist_head *list, struct rir_type *t, struct rir_type *itype)
{
    struct rir_type **subtype;
    bool found = true;
    while (found) {
        found = false;
        unsigned int i = 0;
        darray_foreach(subtype, itype->subtypes) {
            rir_type_purge_from_subtypes(list, t, *subtype);
            if (*subtype == t) {
                found = true;
                break;
            }
            ++i;
        }
        if (found) {
            darray_remove(itype->subtypes, i);
        }
    }
}

void rir_types_list_purge(struct RFilist_head *list, struct rir_type *t)
{
    // iterate all types of the list and delete it if found in their subtypes
    struct rir_type *itype;
    rf_ilist_for_each(list, itype, ln) {
        rir_type_purge_from_subtypes(list, t, itype);
    }
}

void rir_types_list_deinit(struct rir_types_list *l)
{
    struct rir_type *t = rf_ilist_pop(&l->lh, struct rir_type, ln);
    while (t) {
        rir_types_list_purge(&l->lh, t);
        /* rir_type_destroy(t); */
        rir_type_destroy_check_list(t, &l->lh);
        t = rf_ilist_pop(&l->lh, struct rir_type, ln);
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

struct rir_type *rir_types_list_get_type(struct RFilist_head *list,
                                         const struct type *type,
                                         const struct RFstring *name)
{
    struct rir_type *iter_rir_type;

    if (!name && type->category == TYPE_CATEGORY_ELEMENTARY) {
        return &elementary_rir_types_[type_elementary(type)];
    }
    rf_ilist_for_each(list, iter_rir_type, ln) {
        if (rir_type_equals_type(iter_rir_type, type, name)) {
                return iter_rir_type;
        }
    }
    return NULL;
}

// temporary macros to allow visualization of rir type creation. Will go away
/* #define TEMP_RIR_CREATION_TRACE */
#ifdef TEMP_RIR_CREATION_TRACE
#define DDC(...) do { RFS_PUSH(); RF_CDEBUG(__VA_ARGS__); RFS_POP();}while(0)
#else
#define DDC(...)
#endif
/* #define TEMP_RIR_REORDERING_TRACE */
#ifdef TEMP_RIR_REORDERING_TRACE
#define DDR(...) do { RFS_PUSH(); RF_CDEBUG(__VA_ARGS__); RFS_POP();}while(0)
#else
#define DDR(...)
#endif

/**
 * Remove from temporary list and add to rir types list
 * @param remt      The type to remove from one list and add to the other
 * @param tmplist   The list to remove it from
 * @param list      The list to add the type to
 */
static inline void rir_types_list_remtmp_add(struct rir_type *remt,
                                             struct RFilist_head *tmplist,
                                             struct rir_types_list *list)
{
    rf_ilist_delete_from(tmplist, &remt->ln);
    rf_ilist_add_tail(&list->lh, &remt->ln);
}

/**
 *
 * Check if a rir type is in the list and return it if it is.
 *
 * Unfortunately at this stage in the original list some types have been put
 * multiple times and as such here we need to check for type equality and can't
 * just use @ref rf_ilist_has(). Plus we need to return the type itself so that
 * caller code knows which pointer to delete from the big tmp list
 *
 * @param list        The list to check for the type
 * @param t           The type to check for existence in the list
 * @return            Returns either the same type or a type that's considered
 *                    equal to the given type and has been found in the list
 */
struct rir_type *rir_types_list_has(struct RFilist_head *list, struct rir_type *t)
{
    struct rir_type *it;
    rf_ilist_for_each(list, it, ln) {
        if (rir_type_equals(t, it, RIR_TYPECMP_SIMPLE)) {
            return it;
        }
    }
    return false;
}
static void rir_type_add_dependecies_to_list(struct rir_type *t,
                                             struct RFilist_head *tmplist,
                                             struct rir_types_list *l)
{
    struct rir_type **subtype;
    DDR("Iterating subtypes of "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(rir_type_str_or_die(t)));
    darray_foreach(subtype, t->subtypes) {
        struct rir_type *in_tmpl = rir_types_list_has(tmplist, *subtype);
        DDR("Subtype " RF_STR_PF_FMT " %s in the tmp list\n",
            RF_STR_PF_ARG(rir_type_str_or_die(*subtype)), in_tmpl ? "is" : "is not");
        struct rir_type  *in_nrml = rir_types_list_has(&l->lh, *subtype);
        DDR("Subtype " RF_STR_PF_FMT " %s in the normal list\n",
            RF_STR_PF_ARG(rir_type_str_or_die(*subtype)), in_nrml ? "is" : "is not");
        // go further into the type
        rir_type_add_dependecies_to_list(*subtype, tmplist, l);
        // if needed remove type from tmp list and add to new list
        if (in_tmpl && !in_nrml) {
            DDR("Removing subtype "RF_STR_PF_FMT" from tmp and adding to list\n", RF_STR_PF_ARG(rir_type_str_or_die(*subtype)));
            rir_types_list_remtmp_add(in_tmpl, tmplist, l);
        }

    }
}

bool rir_types_list_init(struct rir_types_list *rir_types,
                         struct rf_objset_type *types_set)
{
    struct type *t;
    struct rf_objset_iter it1;
    struct rir_type *iter_rir_type;
    struct rir_type *created_rir_type;
    bool found;
    DDC("iterating types to create rir types list for a module\n\n");
    struct RFilist_head tmpl;
    rf_ilist_head_init(&tmpl);

    rf_objset_foreach(types_set, &it1, t) {
        DDC("iterating type: "RF_STR_PF_FMT"\n",
           RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
        // if this type is a leaf of a defined type, check if the type of the
        // leaf already exists in the types list
        struct type *check_type = t;
        if (t->category == TYPE_CATEGORY_LEAF && t->leaf.type->category == TYPE_CATEGORY_DEFINED) {
            check_type = t->leaf.type;
        }
        // see if this type already has an equivalent rir type
        found = false;
        rf_ilist_for_each(&tmpl, iter_rir_type, ln) {
            if (rir_type_equals_type(iter_rir_type, check_type, NULL)) {
                found = true;
                break;
            }
        }
        // if it does don't bother with it anymore and set the rir type to the found one
        if (found) {
            DDC("Equal rir type already found: "RF_STR_PF_FMT"\n",
                      RF_STR_PF_ARG(rir_type_str_or_die(iter_rir_type)));
            t->rir_type = iter_rir_type;
            continue;
        }

        bool equivalent_in_list;
        created_rir_type = rir_type_find_or_create(t, NULL, &tmpl, &equivalent_in_list);
        DDC("-->created rir type: "RF_STR_PF_FMT"\n",
               RF_STR_PF_ARG(rir_type_str_or_die(created_rir_type)));
        if (!created_rir_type) {
            RF_ERROR("Failed to create a rir type during transition to Refu IR.");
            return false;
        }
        /* if (!rir_types_list_has(&tmpl, created_rir_type)) { */
        if (!equivalent_in_list) {
            rf_ilist_add_tail(&tmpl, &created_rir_type->ln);
            created_rir_type->indexed = true;
        }
    }

    // TODO: This whole thing here smells ... when I find time should improve.
    // Perhaps use an array? Or rethink the existence of rir_types_list.
    // ---
    // now reorder the list and make sure that all types are in order of need
    DDR("\nREORDERING\n\n");
    rf_ilist_head_init(&rir_types->lh);
    iter_rir_type = rf_ilist_pop(&tmpl, struct rir_type, ln);
    while (iter_rir_type) {
        if (rir_type_is_elementary(iter_rir_type)) {
            DDR("Got elementary type "RF_STR_PF_FMT" at reordering. Adding directly to new list\n", RF_STR_PF_ARG(rir_type_str_or_die(iter_rir_type)));
            rf_ilist_add_tail(&rir_types->lh, &iter_rir_type->ln);
            // get next type
            iter_rir_type = rf_ilist_pop(&tmpl, struct rir_type, ln);
            continue;
        }
        // if the type's subtypes/dependencies are not in the list yet add them
        // before adding this type
        rir_type_add_dependecies_to_list(iter_rir_type, &tmpl, rir_types);
        rf_ilist_add_tail(&rir_types->lh, &iter_rir_type->ln);
        // get next type
        iter_rir_type = rf_ilist_pop(&tmpl, struct rir_type, ln);
    }

#ifdef TEMP_RIR_REORDERING_TRACE
    // also if we want to debug stuff print the newly reordered list
    printf("\n>>Newly re-ordered list follows:\n\n");
    rf_ilist_for_each(&rir_types->lh, iter_rir_type, ln) {
        DDR("--> " RF_STR_PF_FMT"\n", RF_STR_PF_ARG(rir_type_str_or_die(iter_rir_type)));
    }
#endif
    return true;
}

struct rir_types_list *rir_types_list_create(struct rf_objset_type *types_set)
{
    struct rir_types_list *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_types_list_init(ret, types_set)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void rir_types_list_destroy(struct rir_types_list *t)
{
    rir_types_list_deinit(t);
    free(t);
}
