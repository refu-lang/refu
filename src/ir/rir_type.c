#include <ir/rir_type.h>

#include <String/rf_str_core.h>

#include <ast/type.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

static bool rir_type_init_iteration(struct rir_type *type, const struct type *input,
                                    const struct RFstring *name,
                                    enum rir_type_category *previous_type_op,
                                    struct rir_type **newly_created_type)
{
    enum rir_type_category op_category;
    struct rir_type *new_type;
    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        if (type_elementary(input) != ELEMENTARY_TYPE_NIL) {
            new_type = rir_type_create(input, name, NULL);
            darray_append(type->subtypes, new_type);
        }
        break;
    case TYPE_CATEGORY_DEFINED:
        type->category = COMPOSITE_RIR_DEFINED;
        type->name = input->defined.name;
        new_type = rir_type_create(input->defined.type, NULL, NULL);
        if (!new_type) {
            return false;
        }
        darray_append(type->subtypes, new_type);
        break;
    case TYPE_CATEGORY_OPERATOR:
        switch (input->operator.type) {
        case TYPEOP_PRODUCT:
            op_category = COMPOSITE_PRODUCT_RIR_TYPE;
            break;
        case TYPEOP_SUM:
            op_category = COMPOSITE_SUM_RIR_TYPE;
            break;
        case TYPEOP_IMPLICATION:
            op_category = COMPOSITE_IMPLICATION_RIR_TYPE;
            break;
        default:
            RF_ASSERT(false, "Illegal type operation encountered");
            return false;
        }

        if (*previous_type_op == RIR_TYPE_CATEGORY_COUNT) {
            *previous_type_op = op_category;
            new_type = type;
            new_type->category = op_category;
        } else if (*previous_type_op != op_category) {
            new_type = rir_type_create(input, NULL, NULL);
            new_type->category = op_category;
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        } else {
            new_type = type;
            new_type->category = op_category;
        }

        if (!rir_type_init_iteration(new_type, input->operator.left, NULL, previous_type_op, newly_created_type)) {
            return false;
        }

        if (*previous_type_op != op_category) {
            new_type = rir_type_create(input->operator.right, NULL, NULL);
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        }

        if (!rir_type_init_iteration(new_type, input->operator.right, NULL, previous_type_op, newly_created_type)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        if (!rir_type_init_iteration(type, input->leaf.type, input->leaf.id, previous_type_op, newly_created_type)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_GENERIC:
        RF_ASSERT(false, "Generic types not supported in the IR yet");
        break;
    }
    return true;
}

bool rir_type_init_before_iteration(struct rir_type *type,
                                    const struct type *input,
                                    const struct RFstring *name)
{
    darray_init(type->subtypes);
    type->name = name;
    type->indexed = false;

    // for elementary types there is nothing to inialize
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        type->category = (enum rir_type_category) type_elementary(input);
        return false;
    } else if (input->category == TYPE_CATEGORY_LEAF &&
        input->leaf.type->category == TYPE_CATEGORY_ELEMENTARY) {
        // for leafs, that are elementary types, just create a named elementary rir type
        type->category = (enum rir_type_category) type_elementary(input->leaf.type);
        type->name = input->leaf.id;
        return false;
    }
    return true;
}

bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name,
                   struct rir_type **newly_created_type)
{

    if (!rir_type_init_before_iteration(type, input, name)) {
        return true; // no need to iterate children of input type
    }
    // iterate the subtypes
    enum rir_type_category previous_type_op = RIR_TYPE_CATEGORY_COUNT;
    if (!rir_type_init_iteration(type, input, name, &previous_type_op, newly_created_type)) {
        return false;
    }

    return true;
}

struct rir_type *rir_type_alloc()
{
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    return ret;
}

struct rir_type *rir_type_create(const struct type *input,
                                 const struct RFstring *name,
                                 struct rir_type **newly_created_type)
{
    struct rir_type *ret = rir_type_alloc();
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input, name, newly_created_type)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }

    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    free(t);
}
void rir_type_destroy(struct rir_type *t)
{
    rir_type_deinit(t);
    rir_type_dealloc(t);
}
void rir_type_deinit(struct rir_type *t)
{
    struct rir_type **subtype;
    darray_foreach(subtype, t->subtypes) {
        if (!(*subtype)->indexed) {
            rir_type_destroy(*subtype);
        }
    }

    darray_free(t->subtypes);
}

bool rir_type_equals(struct rir_type *a, struct rir_type *b)
{
    struct rir_type **subtype_a = NULL;
    unsigned int i = 0;
    if (a->category != b->category) {
        return false;
    }

    if ((a->name == NULL &&  b->name != NULL) ||
        (a->name != NULL && b->name == NULL)) {
        return false;
    } else if (a->name != NULL && b->name != NULL &&
               !rf_string_equal(a->name, b->name)) {
        return false;
    }

    if (darray_size(a->subtypes) != darray_size(b->subtypes)) {
        return false;
    }

    darray_foreach(subtype_a, a->subtypes) {
        if (!rir_type_equals(*subtype_a, darray_item(b->subtypes, i))) {
            return false;
        }
        ++i;
    }

    return true;
}

static bool rir_type_equals_typeop_type_do(struct rir_type *r_type,
                                           unsigned int *index,
                                           struct type *op_type)
{
    unsigned int old_index;
    if (op_type->category == TYPE_CATEGORY_OPERATOR) {
        switch (op_type->operator.type) {
        case TYPEOP_PRODUCT:
            if (r_type->category != COMPOSITE_PRODUCT_RIR_TYPE) {
                return false;
            }
            break;
        case TYPEOP_SUM:
            if (r_type->category != COMPOSITE_SUM_RIR_TYPE) {
                return false;
            }
            break;
        case TYPEOP_IMPLICATION:
            if (r_type->category != COMPOSITE_IMPLICATION_RIR_TYPE) {
                return false;
            }
            break;
        default:
            RF_ASSERT(false, "Illegal type operation encountered");
            return false;
            break;
        }


        if (darray_size(r_type->subtypes) >= (*index) + 1) {
            return false;
        }
        old_index = *index;
        *index = *index + 1;
        if (rir_type_with_index_equals_type(darray_item(r_type->subtypes, old_index), index, op_type->operator.left)) {
            return false;
        }

        if (darray_size(r_type->subtypes) >= (*index) + 1) {
            return false;
        }
        old_index = *index;
        *index = *index + 1;
        if (rir_type_with_index_equals_type(darray_item(r_type->subtypes, old_index), index, op_type->operator.right)) {
            return false;
        }
    }

    RF_ASSERT(false, "Should never get here");
    return false;
}

bool rir_type_equals_type(struct rir_type *r_type, struct type *n_type)
{
    unsigned int index = 0;
    return rir_type_with_index_equals_type(r_type, &index, n_type);
}

bool rir_type_is_subtype_of_other(struct rir_type *t,
                                  struct rir_type *other)
{
    struct rir_type **subtype;
    darray_foreach(subtype, other->subtypes) {
        if (*subtype == t) {
            return true;
        }
        if (darray_size((*subtype)->subtypes) != 0) {
            if (rir_type_is_subtype_of_other(t, *subtype)) {
                return true;
            }
        }
    }
    return false;
}

bool rir_type_with_index_equals_type(struct rir_type *r_type, unsigned int *index, struct type *n_type)
{
    if (rir_type_is_elementary(r_type)) {
        if (n_type->category == TYPE_CATEGORY_LEAF) {
            if (r_type->name == n_type->leaf.id &&
                rir_type_with_index_equals_type(r_type, index, n_type->leaf.type)) {
                return true;
            }
        }
        if (n_type->category != TYPE_CATEGORY_ELEMENTARY) {
            return false;
        }
        return (enum elementary_type)r_type->category == type_elementary(n_type);
    }

    if (n_type->category != TYPE_CATEGORY_OPERATOR) {
        return false;
    }

    return rir_type_equals_typeop_type_do(r_type, index, n_type);
}

const struct RFstring *rir_type_get_nth_name(struct rir_type *t, unsigned n)
{
    if (darray_size(t->subtypes) <= n) {
        RF_ERROR("Requested rir_type name of subtype out of bounds");
        return NULL;
    }
    return ((struct rir_type*)darray_item(t->subtypes, n))->name;
}

const struct rir_type *rir_type_get_nth_type(struct rir_type *t, unsigned n)
{
    if (darray_size(t->subtypes) <= n) {
        RF_ERROR("Requested rir_type type of subtype out of bounds");
        return NULL;
    }
    return darray_item(t->subtypes, n);
}
// very very temporary macro to allow visualization of rir type creation. Will go away
// #define TEMP_RIR_DEBUG 1
bool rir_create_types(struct RFilist_head *rir_types, struct RFilist_head *composite_types)
{
    struct type *t;
    struct rir_type *iter_rir_type;
    struct rir_type *created_rir_type;
    bool found;
    rf_ilist_head_init(rir_types);
    rf_ilist_for_each(composite_types, t, lh) {
#if TEMP_RIR_DEBUG
        RFS_buffer_push();
        printf("iterating type: "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str(t, true)));
        fflush(stdout);
#endif
        // first of all see if this composite type already has an equivalent rir type
        found = false;
        rf_ilist_for_each(rir_types, iter_rir_type, ln) {
            if (rir_type_equals_type(iter_rir_type, t)) {
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
        rf_ilist_add_tail(rir_types, &created_rir_type->ln);
        created_rir_type->indexed = true;
    }

    return true;
}

static inline const struct RFstring *rir_type_op_to_str(const struct rir_type *t)
{
    switch(t->category) {
    case COMPOSITE_PRODUCT_RIR_TYPE:
        return type_op_str(TYPEOP_PRODUCT);
    case COMPOSITE_SUM_RIR_TYPE:
        return type_op_str(TYPEOP_SUM);
    case COMPOSITE_IMPLICATION_RIR_TYPE:
        return type_op_str(TYPEOP_IMPLICATION);
    default:
        RF_ASSERT(false, "Invalid rir_type at rir_type_op_to_str()");
        break;
    }

    return NULL;
}

static const struct RFstring rir_op_names[] = {
    [COMPOSITE_PRODUCT_RIR_TYPE] = RF_STRING_STATIC_INIT("product_type"),
    [COMPOSITE_SUM_RIR_TYPE] = RF_STRING_STATIC_INIT("sum_type"),
    [COMPOSITE_IMPLICATION_RIR_TYPE] = RF_STRING_STATIC_INIT("implication_type"),
};

const struct RFstring *rir_type_str(const struct rir_type *t)
{
    struct RFstring *s;
    struct rir_type **subtype;
    unsigned int count = 1;
    if (rir_type_is_elementary(t)) {
        return t->name
            ? RFS_(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                   RF_STR_PF_ARG(t->name),
                   RF_STR_PF_ARG(type_elementary_get_str((enum elementary_type)t->category)))
            : RFS_(RF_STR_PF_FMT, RF_STR_PF_ARG(type_elementary_get_str((enum elementary_type)t->category)));
    }

    switch(t->category) {
    case COMPOSITE_PRODUCT_RIR_TYPE:
    case COMPOSITE_SUM_RIR_TYPE:
    case COMPOSITE_IMPLICATION_RIR_TYPE:
        s = t->name
            ? RFS_(RF_STR_PF_FMT ":" RF_STR_PF_FMT"( ", RF_STR_PF_ARG(t->name),
                   RF_STR_PF_ARG(&rir_op_names[t->category]))
            : RFS_(RF_STR_PF_FMT"( ", RF_STR_PF_ARG(&rir_op_names[t->category]));
        darray_foreach(subtype, t->subtypes) {
            s = darray_size(t->subtypes) == count
              ? RFS_(RF_STR_PF_FMT RF_STR_PF_FMT,
                     RF_STR_PF_ARG(s),
                     RF_STR_PF_ARG(rir_type_str(*subtype)))
              : RFS_(RF_STR_PF_FMT RF_STR_PF_FMT RF_STR_PF_FMT,
                     RF_STR_PF_ARG(s),
                     RF_STR_PF_ARG(rir_type_str(*subtype)),
                     RF_STR_PF_ARG(rir_type_op_to_str(t)));
            ++ count;
        }
        return RFS_(RF_STR_PF_FMT " )", RF_STR_PF_ARG(s));
    case COMPOSITE_RIR_DEFINED:

        return RFS_("type "RF_STR_PF_FMT"{ " RF_STR_PF_FMT " }",
                    RF_STR_PF_ARG(t->name),
                    RF_STR_PF_ARG(rir_type_str(darray_item(t->subtypes, 0))));
    default:
        RF_ASSERT(false, "Invalid rir_type at rir_type_str()");
        break;
    }
}

i_INLINE_INS bool rir_type_is_elementary(const struct rir_type *t);
