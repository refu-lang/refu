#include <ir/rir_argument.h>
#include <ir/rir.h>
#include <ir/rir_typedef.h>
#include <ir/rir_type.h>
#include <Data_Structures/darray.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>

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

static void rir_ltype_comp_init(struct rir_ltype *t, const struct rir_typedef *def)
{
    t->category = RIR_LTYPE_COMPOSITE;
    t->tdef = def;
}

struct rir_ltype *rir_ltype_comp_create(struct rir_typedef *def)
{
    struct rir_ltype *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_ltype_comp_init(ret, def);
    return ret;
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
    RF_ASSERT(darray_size(type->subtypes) == 0,
              "This function should only be called with elementary rir type");
    RF_ASSERT(type->name, "An elementary type should always be accompanied by a name");
    rir_ltype_elem_init(&a->type, (enum elementary_type)type->category);
    a->name = type->name;
}

struct rir_argument *rir_argument_create(const struct rir_type *t)
{
    struct rir_argument *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_argument_init(ret, t);
    return ret;
}

struct rir_argument *rir_argument_create_from_typedef(const struct rir_typedef *d)
{
    struct rir_argument *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->name = d->name;
    rir_ltype_comp_init(&ret->type, d);
    return ret;
}

void rir_argument_destroy(struct rir_argument *a)
{
    free(a);
}

bool rir_argument_tostring(struct rir *r, const struct rir_argument *arg)
{
    bool ret = true;
    if (arg->type.category == RIR_LTYPE_ELEMENTARY) {
        ret = rf_stringx_append(
            r->buff,
            RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                RF_STR_PF_ARG(arg->name),
                RF_STR_PF_ARG(type_elementary_get_str(arg->type.etype))));
    } else {
        rf_stringx_append(r->buff, arg->type.tdef->name);
    }


    return ret;
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
        if (!rir_type_is_trivial(type)) {
            if (!(arg = rir_argument_create(type))) {
                return false;
            }
            darray_append(*arr, arg);
        }
    } else if (type->category != COMPOSITE_IMPLICATION_RIR_TYPE) {
        darray_foreach(subtype, type->subtypes) {
            if (!rir_type_is_trivial(*subtype)) {
                if (!(arg = rir_argument_create(*subtype))) {
                    return false;
                }
                darray_append(*arr, arg);
            }
        }
    }
    return true;
}

bool rir_argsarr_tostring(struct rir *r, const struct args_arr *arr)
{
    size_t i = 0;
    size_t args_num = darray_size(*arr);
    const struct rir_argument **arg;
    darray_foreach(arg, *arr) {
        if (!rir_argument_tostring(r, *arg)) {
            return false;
        }
        if (++i != args_num) {
            if (!rf_stringx_append_cstr(r->buff, ", ")) {
                return false;
            }
        }
    }
    return true;
}
