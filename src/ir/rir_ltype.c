#include <ir/rir_ltype.h>
#include <ir/rir.h>
#include <ir/rir_typedef.h>
#include <ir/rir_object.h>
#include <types/type.h>
#include <ast/function.h>

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

void rir_ltype_elem_init(struct rir_ltype *t, enum elementary_type etype)
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

void rir_ltype_comp_init(struct rir_ltype *t, const struct rir_typedef *def, bool is_pointer)
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

struct rir_ltype *rir_ltype_create_from_type(const struct type *t, struct rir_ctx *ctx)
{
    if (t->category == TYPE_CATEGORY_ELEMENTARY) {
        return rir_ltype_elem_create(t->elementary.etype, false);
    } else if (t->category == TYPE_CATEGORY_DEFINED) {
        struct rir_object *tdef_obj = rir_ctx_st_getobj(ctx, type_defined_get_name(t));
        if (!tdef_obj) {
            RF_ERROR("Could not find typedef identifier RIR object in symbol table");
            return NULL;
        }
        return rir_ltype_comp_create(&tdef_obj->tdef, false);
    } else {
        RF_CRITICAL_FAIL("Unexpected type category");
        return NULL;
    }
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

const struct rir_ltype *rir_ltype_comp_member_type(const struct rir_ltype *t, uint32_t idx)
{
    RF_ASSERT(rir_ltype_is_composite(t), "Expected composite type");
    const struct rir_argument *arg = rir_typedef_argat(t->tdef, idx);
    return arg ? &arg->type : NULL;
}

int rir_ltype_union_matched_type_from_fncall(const struct rir_ltype *t, const struct ast_node *n, struct rir_ctx *ctx)
{
    RF_ASSERT(rir_ltype_is_union(t), "Expected a union type");
    const struct type *matched = ast_fncall_params_type(n);
    struct rir_object **arg;
    struct args_arr t_args;
    if (!rir_type_to_arg_array(type_get_rir_or_die(matched), &t_args, ctx)) {
        return -1;
    }

    int index = 0;
    darray_foreach(arg, t->tdef->arguments_list) {
        RF_ASSERT((*arg)->arg.type.category == RIR_LTYPE_COMPOSITE,
                  "each of the union's members should be its own typedef");
        //now check if this is the type that matches the function call
        if (rir_argsarr_equal(&t_args, &(*arg)->arg.type.tdef->arguments_list)) {
            goto end;
        }
        ++index;
    }
    // failure
    index = -1;

end:
    rir_argsarr_deinit(&t_args, ctx);
    return index;
}
