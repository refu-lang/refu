#include <ir/rir_argument.h>
#include <ir/rir_type.h>
#include <Data_Structures/darray.h>
#include <Utils/memory.h>

static void rir_ltype_elem_init(struct rir_ltype *t, enum elementary_type etype)
{
    t->category = RIR_LTYPE_ELEMENTARY;
    t->etype = etype;
}

struct rir_ltype *rir_ltype_elem_create(enum elementary_type etype)
{
    struct rir_ltype *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_ltype_elem_init(ret, etype);
    return ret;
}

static void rir_ltype_comp_init(struct rir_ltype *t, const struct rir_type *ctype)
{
    t->category = RIR_LTYPE_COMPOSITE;
    t->ctype = ctype;
}

struct rir_ltype *rir_ltype_comp_create(struct rir_type *ctype)
{
    struct rir_ltype *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_ltype_comp_init(ret, ctype);
    return ret;
}

struct rir_ltype *rir_ltype_from_rtype(struct rir_type *type)
{
    if (darray_size(type->subtypes) == 0) {
        return rir_ltype_elem_create((enum elementary_type)type->category);
    }
    return rir_ltype_comp_create(type);
}

void rir_ltype_destroy(struct rir_ltype *t)
{
    free(t);
}

size_t rir_ltype_bytesize(const struct rir_ltype *a)
{
    // TOO
    return 4;
}


static void rir_argument_init(struct rir_argument *a, const struct rir_type *type)
{
    if (darray_size(type->subtypes) == 0) {
        rir_ltype_elem_init(&a->type, (enum elementary_type)type->category);
    } else {
        rir_ltype_comp_init(&a->type, type);
    }
    a->name = type->name;
}

struct rir_argument *rir_argument_create(const struct rir_type *t)
{
    struct rir_argument *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_argument_init(ret, t);
    return ret;
}

void rir_argument_destroy(struct rir_argument *a)
{
    free(a);
}

bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr)
{
    RF_ASSERT(type->category != COMPOSITE_RIR_DEFINED ||
              type->category != COMPOSITE_SUM_RIR_TYPE ||
              type->category != COMPOSITE_IMPLICATION_RIR_TYPE,
              "Called with illegal rir type");
    struct rir_type **subtype;
    struct rir_argument *arg;
    darray_init(*arr);
    if (darray_size(type->subtypes) == 0) {
        if (!rir_type_is_category(type, ELEMENTARY_RIR_TYPE_NIL)) {
            if (!(arg = rir_argument_create(type))) {
                return false;
            }
            darray_append(*arr, arg);
        }
    } else {
        darray_foreach(subtype, type->subtypes) {
            if (!rir_type_is_category(type, ELEMENTARY_RIR_TYPE_NIL)) {
                if (!(arg = rir_argument_create(*subtype))) {
                    return false;
                }
                darray_append(*arr, arg);
            }
        }
    }
    return true;
}
