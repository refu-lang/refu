#include <ir/rir_type.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

static struct rir_type i_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)                           \
    [i_type] = {.category = i_type}

    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_RIR_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};



static bool rir_type_init_iteration(struct rir_type *type, const struct type *input,
                                    const struct RFstring *name)
{
    // for now here we assume it's only product types
    struct rir_type *new_type;
    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        if (type_elementary(input) != ELEMENTARY_TYPE_NIL) {
            new_type = rir_type_alloc(input);
            new_type->name = name;
            darray_append(type->subtypes, new_type);
        }
        break;
    case TYPE_CATEGORY_DEFINED:
        //TODO
        RF_ASSERT(false, "Not implemented yet");
        break;
    case TYPE_CATEGORY_OPERATOR:
        // TODO: make it work with sum types too
        switch (input->operator.type) {
        case TYPEOP_PRODUCT:
            type->category = COMPOSITE_PRODUCT_RIR_TYPE;
            break;
        case TYPEOP_SUM:
            type->category = COMPOSITE_SUM_RIR_TYPE;
            break;
        case TYPEOP_IMPLICATION:
            type->category = COMPOSITE_IMPLICATION_RIR_TYPE;
            break;
        default:
            RF_ASSERT(false, "Illegal type operation encountered");
            return false;
        }

        if (!rir_type_init_iteration(type, input->operator.left, NULL)) {
            return false;
        }
        if (!rir_type_init_iteration(type, input->operator.right, NULL)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        if (!rir_type_init_iteration(type, input->leaf.type, input->leaf.id)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_GENERIC:
        RF_ASSERT(false, "Generic types not supported in the IR yet");
        break;
    }
    return true;
}

bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name)
{
    darray_init(type->subtypes);

    // for elementary types there is nothing to inialize
    if (rir_type_is_elementary(type)) {
        return true;
    }

    if (!rir_type_init_iteration(type, input, name)) {
        return false;
    }

    return true;
}

struct rir_type *rir_type_alloc(const struct type *input)
{
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        return &i_elementary_types[type_elementary(input)];
    }

    // else
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    // will not necessarily be a product type but this definitely marks it as a composite type
    ret->category = COMPOSITE_PRODUCT_RIR_TYPE;
    return ret;

}

struct rir_type *rir_type_create(const struct type *input, const struct RFstring *name)
{
    struct rir_type *ret = rir_type_alloc(input);
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input, name)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }

    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    if (!rir_type_is_elementary(t)) {
        free(t);
    }
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
        rir_type_destroy(*subtype);
    }

    darray_free(t->subtypes);
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

bool rir_create_types(struct RFilist_head *rir_types, struct RFilist_head *composite_types)
{
    struct type *t;
    struct rir_type *created_rir_type;
    rf_ilist_head_init(rir_types);

    rf_ilist_for_each(composite_types, t, lh) {
        created_rir_type = rir_type_create(t, NULL);
        if (!created_rir_type) {
            RF_ERROR("Failed to create a rir type during transition Refu IR.");
            return false;
        }
        rf_ilist_add_tail(rir_types, &created_rir_type->ln);
    }

    return true;
}

i_INLINE_INS bool rir_type_is_elementary(struct rir_type *t);
