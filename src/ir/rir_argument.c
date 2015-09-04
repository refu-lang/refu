#include <ir/rir_argument.h>
#include <ir/rir.h>
#include <ir/rir_typedef.h>
#include <ir/rir_type.h>
#include <types/type.h>
#include <ast/function.h>
#include <Data_Structures/darray.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static struct rir_ltype elementary_types[] = {
#define RIR_LTYPE_ELEMINIT(i_type_)                                     \
    [i_type_] = {.category = RIR_LTYPE_ELEMENTARY, .etype = i_type_, .is_pointer = false}
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_INT_8),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_8),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_INT_16),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_16),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_INT_32),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_32),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_INT_64),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_64),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_INT),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_UINT),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_FLOAT_32),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_FLOAT_64),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_STRING),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_BOOL),
    RIR_LTYPE_ELEMINIT(ELEMENTARY_TYPE_NIL)
#undef RIR_LTYPE_ELEMINIT
};

static struct rir_ltype elementary_ptr_types[] = {
#define RIR_LTYPE_PTRELEMINIT(i_type_)                                     \
    [i_type_] = {.category = RIR_LTYPE_ELEMENTARY, .etype = i_type_, .is_pointer = true}
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_8),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_8),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_16),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_16),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_32),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_32),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_64),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_64),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_FLOAT_32),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_FLOAT_64),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_STRING),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_BOOL),
    RIR_LTYPE_PTRELEMINIT(ELEMENTARY_TYPE_NIL)
#undef RIR_LTYPE_PTRELEMINIT
};

i_INLINE_INS bool rir_ltype_is_elementary(const struct rir_ltype *t);
i_INLINE_INS bool rir_ltype_is_composite(const struct rir_ltype *t);
bool rir_ltype_is_union(const struct rir_ltype *t)
{
    return t->category == RIR_LTYPE_COMPOSITE && t->tdef->is_union;
}

static void rir_ltype_elem_init(struct rir_ltype *t, enum elementary_type etype)
{
    t->category = RIR_LTYPE_ELEMENTARY;
    t->etype = etype;
    t->is_pointer = false;
}

struct rir_ltype *rir_ltype_elem_create_from_string(const struct RFstring *name, bool is_pointer)
{
    enum elementary_type etype = type_elementary_from_str(name);
    return etype == ELEMENTARY_TYPE_TYPES_COUNT ? NULL : rir_ltype_elem_create(etype, is_pointer);
}

struct rir_ltype *rir_ltype_elem_create(enum elementary_type etype, bool is_pointer)
{
    return is_pointer ? &elementary_ptr_types[etype] : &elementary_types[etype];
}

static void rir_ltype_comp_init(struct rir_ltype *t, const struct rir_typedef *def, bool is_pointer)
{
    t->category = RIR_LTYPE_COMPOSITE;
    t->tdef = def;
    t->is_pointer = is_pointer;
}

struct rir_ltype *rir_ltype_comp_create(const struct rir_typedef *def, bool is_pointer)
{
    struct rir_ltype *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_ltype_comp_init(ret, def, is_pointer);
    return ret;
}

void rir_ltype_destroy(struct rir_ltype *t)
{
    if (t->category != RIR_LTYPE_ELEMENTARY) {
        free(t);
    }
}

struct rir_ltype *rir_ltype_create_from_other(const struct rir_ltype *other, bool is_pointer)
{
    if (other->category == RIR_LTYPE_ELEMENTARY) {
        return rir_ltype_elem_create(other->etype, is_pointer);
    } else { // composite
        return rir_ltype_comp_create(other->tdef, is_pointer);
    }
}

struct rir_ltype *rir_ltype_copy_from_other(const struct rir_ltype *other)
{
    if (other->category == RIR_LTYPE_ELEMENTARY) {
        return rir_ltype_elem_create(other->etype, other->is_pointer);
    } else { // composite
        return rir_ltype_comp_create(other->tdef, other->is_pointer);
    }
}

bool rir_ltype_equal(const struct rir_ltype *a, const struct rir_ltype *b)
{
    if (a->category != b->category) {
        return false;
    }
    if (a->is_pointer != b->is_pointer) {
        return false;
    }

    if (a->category == RIR_LTYPE_ELEMENTARY) {
        if (a->etype != b->etype) {
            return false;
        }
    } else { // composite type
        if (!rir_typedef_equal(a->tdef, b->tdef)) {
            return false;
        }
    }
    return true;
}

size_t rir_ltype_bytesize(const struct rir_ltype *a)
{
    // TOO
    return 4;
}

const struct RFstring *rir_ltype_string(const struct rir_ltype *t)
{
    if (t->category == RIR_LTYPE_ELEMENTARY) {
        return RFS(RF_STR_PF_FMT"%s",
                   RF_STR_PF_ARG(type_elementary_get_str(t->etype)),
                   t->is_pointer ? "*" : "");
    } else {
        return RFS(RF_STR_PF_FMT"%s",
                   RF_STR_PF_ARG((t->tdef->name)),
                   t->is_pointer ? "*" : "");
    }
}

const struct rir_ltype *rir_ltype_comp_member_type(const struct rir_ltype *t, unsigned int i)
{
    RF_ASSERT(rir_ltype_is_composite(t), "Expected composite type");
    const struct rir_argument *arg = rir_typedef_argat(t->tdef, i);
    return arg ? &arg->type : NULL;
}

int rir_ltype_union_matched_type_from_fncall(const struct rir_ltype *t, const struct ast_node *n, const struct rir *r)
{
    RF_ASSERT(rir_ltype_is_union(t), "Expected a union type");
    const struct type *matched = ast_fncall_params_type(n);
    struct rir_argument **arg;
    struct args_arr t_args;
    if (!rir_type_to_arg_array(type_get_rir_or_die(matched), &t_args, r)) {
        return -1;
    }

    int index = 0;
    darray_foreach(arg, t->tdef->arguments_list) {
        RF_ASSERT((*arg)->type.category == RIR_LTYPE_COMPOSITE,
                  "each of the union's members should be its own typedef");
        //now check if this is the type that matches the function call
        if (rir_argsarr_equal(&t_args, &(*arg)->type.tdef->arguments_list)) {
            goto end;
        }
        ++index;
    }
    // failure
    index = -1;

end:
    rir_argsarr_deinit(&t_args);
    return index;
}



static void rir_argument_init(struct rir_argument *a, const struct rir_type *type, const struct rir *r)
{
    static const struct RFstring noname = RF_STRING_STATIC_INIT("noname");
    if (rir_type_is_elementary(type)) {
        rir_ltype_elem_init(&a->type, (enum elementary_type)type->category);
        if (type->name) {
            a->name = type->name;
        } else {
            a->name = &noname;
        }
    } else {
        const struct RFstring *s = type_get_unique_type_str(type->type, true);
        struct rir_typedef *def = rir_typedef_byname(r, s);
        RF_ASSERT_OR_EXIT(def, "typedef should have been found by name");
        rir_ltype_comp_init(&a->type, def, false);
        a->name = rf_string_create("GENERATED_NAME");
    }
}

struct rir_argument *rir_argument_create(const struct rir_type *t, const struct rir *r)
{
    struct rir_argument *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_argument_init(ret, t, r);
    return ret;
}

struct rir_argument *rir_argument_create_from_typedef(const struct rir_typedef *d)
{
    struct rir_argument *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->name = d->name;
    rir_ltype_comp_init(&ret->type, d, false);
    return ret;
}

void rir_argument_destroy(struct rir_argument *a)
{
    free(a);
}

bool rir_argument_tostring(struct rirtostr_ctx *ctx, const struct rir_argument *arg)
{
    bool ret = true;
    if (arg->type.category == RIR_LTYPE_ELEMENTARY) {
        ret = rf_stringx_append(
            ctx->rir->buff,
            RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                RF_STR_PF_ARG(arg->name),
                RF_STR_PF_ARG(type_elementary_get_str(arg->type.etype))));
    } else {
        rf_stringx_append(ctx->rir->buff, arg->type.tdef->name);
    }

    return ret;
}

bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr, const struct rir *r)
{
    RF_ASSERT(type->category != COMPOSITE_IMPLICATION_RIR_TYPE,
              "Called with illegal rir type");
    struct rir_type **subtype;
    struct rir_argument *arg;
    darray_init(*arr);
    if (darray_size(type->subtypes) == 0) {
        if (!rir_type_is_trivial(type)) {
            if (!(arg = rir_argument_create(type, r))) {
                return false;
            }
            darray_append(*arr, arg);
        }
    } else if (type->category != COMPOSITE_IMPLICATION_RIR_TYPE) {
        darray_foreach(subtype, type->subtypes) {
            if (!rir_type_is_trivial(*subtype)) {
                if (!(arg = rir_argument_create(*subtype, r))) {
                    return false;
                }
                darray_append(*arr, arg);
            }
        }
    }
    return true;
}

bool rir_argsarr_tostring(struct rirtostr_ctx *ctx, const struct args_arr *arr)
{
    size_t i = 0;
    size_t args_num = darray_size(*arr);
    struct rir_argument **arg;
    darray_foreach(arg, *arr) {
        if (!rir_argument_tostring(ctx, *arg)) {
            return false;
        }
        if (++i != args_num) {
            if (!rf_stringx_append_cstr(ctx->rir->buff, ", ")) {
                return false;
            }
        }
    }
    return true;
}

bool rir_argsarr_equal(const struct args_arr *arr1, const struct args_arr *arr2)
{
    size_t size = darray_size(*arr1);
    if (size != darray_size(*arr2)) {
        return false;
    }

    for (int i = 0; i < size; ++i) {
        if (!rir_ltype_equal(&darray_item(*arr1, i)->type, &darray_item(*arr2, i)->type)) {
            return false;
        }
    }
    return true;
}

void rir_argsarr_deinit(struct args_arr *arr)
{
    struct rir_argument **arg;
    darray_foreach(arg, *arr) {
        rir_argument_destroy(*arg);
    }
    darray_free(*arr);
}
