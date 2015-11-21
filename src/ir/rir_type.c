#include <ir/rir_type.h>
#include <ir/rir.h>
#include <ir/rir_typedef.h>
#include <ir/rir_object.h>
#include <types/type.h>
#include <ast/function.h>
#include <Utils/fixed_memory_pool.h>

static struct rir_type elementary_types[] = {
#define RIR_TYPE_ELEMINIT(i_type_)                                     \
    [i_type_] = {.category = RIR_TYPE_ELEMENTARY, .etype = i_type_, .is_pointer = false}
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_INT_8),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_8),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_INT_16),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_16),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_INT_32),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_32),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_INT_64),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_UINT_64),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_INT),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_UINT),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_FLOAT_32),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_FLOAT_64),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_STRING),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_BOOL),
    RIR_TYPE_ELEMINIT(ELEMENTARY_TYPE_NIL)
#undef RIR_TYPE_ELEMINIT
};

static struct rir_type elementary_ptr_types[] = {
#define RIR_TYPE_PTRELEMINIT(i_type_)                                     \
    [i_type_] = {.category = RIR_TYPE_ELEMENTARY, .etype = i_type_, .is_pointer = true}
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_8),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_8),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_16),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_16),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_32),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_32),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT_64),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT_64),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_INT),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_UINT),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_FLOAT_32),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_FLOAT_64),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_STRING),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_BOOL),
    RIR_TYPE_PTRELEMINIT(ELEMENTARY_TYPE_NIL)
#undef RIR_TYPE_PTRELEMINIT
};

i_INLINE_INS bool rir_type_is_elementary(const struct rir_type *t);
i_INLINE_INS bool rir_type_is_specific_elementary(const struct rir_type *t,
                                                  enum elementary_type etype);
i_INLINE_INS bool rir_type_is_composite(const struct rir_type *t);
bool rir_type_is_union(const struct rir_type *t)
{
    return t->category == RIR_TYPE_COMPOSITE && t->tdef->is_union;
}

void rir_type_elem_init(struct rir_type *t, enum elementary_type etype)
{
    t->category = RIR_TYPE_ELEMENTARY;
    t->etype = etype;
    t->is_pointer = false;
}

struct rir_type *rir_type_alloc(struct rir *r)
{
    struct rir_type *ret = rf_fixed_memorypool_alloc_element(r->rir_types_pool);
    RF_STRUCT_ZERO(ret);
    return ret;
}

void rir_type_destroy(struct rir_type *t, struct rir *r)
{
    if (t->category != RIR_TYPE_ELEMENTARY) {
        rf_fixed_memorypool_free_element(r->rir_types_pool, t);
    }
}

struct rir_type *rir_type_elem_create_from_string(const struct RFstring *name, bool is_pointer)
{
    enum elementary_type etype = type_elementary_from_str(name);
    return etype == ELEMENTARY_TYPE_TYPES_COUNT ? NULL : rir_type_elem_create(etype, is_pointer);
}

struct rir_type *rir_type_elem_create(enum elementary_type etype, bool is_pointer)
{
    return is_pointer ? &elementary_ptr_types[etype] : &elementary_types[etype];
}

void rir_type_comp_init(struct rir_type *t, const struct rir_typedef *def, bool is_pointer)
{
    t->category = RIR_TYPE_COMPOSITE;
    t->tdef = def;
    t->is_pointer = is_pointer;
}

struct rir_type *rir_type_comp_create(const struct rir_typedef *def, struct rir *r, bool is_pointer)
{
    struct rir_type *ret = rir_type_alloc(r);
    if (!ret) {
        return NULL;
    }
    rir_type_comp_init(ret, def, is_pointer);
    return ret;
}

struct rir_type *rir_type_create_from_type(const struct type *t, struct rir_ctx *ctx)
{
    struct rir_typedef *tdef;
    if (t->category == TYPE_CATEGORY_ELEMENTARY) {
        return rir_type_elem_create(t->elementary.etype, false);
    } else if (t->category == TYPE_CATEGORY_DEFINED) {
        struct rir_object *tdef_obj = rir_ctx_st_getobj(ctx, type_defined_get_name(t));
        if (!tdef_obj) {
            RF_ERROR("Could not find typedef identifier RIR object in symbol table");
            return NULL;
        }
        tdef = rir_object_get_typedef(tdef_obj);
        if (!tdef) {
            RF_ERROR("Could not retrieve typedef from rir object. Invalid rir object?");
            return NULL;
        }
        return rir_type_comp_create(tdef, ctx->rir, false);
    } else if (t->category == TYPE_CATEGORY_OPERATOR) {
        struct rir_object *obj = rir_ctx_st_getobj(ctx, type_get_unique_type_str(t));
        if (!obj) {
            RF_ERROR("Could not find operator type, equivalent typedef RIR object in symbol table");
            return NULL;
        }
        tdef = rir_object_get_typedef(obj);
        if (!tdef) {
            RF_ERROR("Could not retrieve typedef from rir object. Invalid rir object?");
            return NULL;
        }
        return rir_type_comp_create(tdef, ctx->rir, false);
    } else {
        RF_CRITICAL_FAIL("Unexpected type category");
        return NULL;
    }
}

struct rir_type *rir_type_create_from_other(const struct rir_type *other, struct rir *r, bool is_pointer)
{
    if (other->category == RIR_TYPE_ELEMENTARY) {
        return rir_type_elem_create(other->etype, is_pointer);
    } else { // composite
        return rir_type_comp_create(other->tdef, r, is_pointer);
    }
}

struct rir_type *rir_type_copy_from_other(const struct rir_type *other, struct rir *r)
{
    if (other->category == RIR_TYPE_ELEMENTARY) {
        return rir_type_elem_create(other->etype, other->is_pointer);
    } else { // composite
        return rir_type_comp_create(other->tdef, r, other->is_pointer);
    }
}

bool rir_type_equal(const struct rir_type *a, const struct rir_type *b)
{
    if (a->category != b->category) {
        return false;
    }
    if (a->category == RIR_TYPE_ELEMENTARY) {
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

bool rir_type_identical(const struct rir_type *a, const struct rir_type *b)
{
    if (a->is_pointer != b->is_pointer) {
        return false;
    }
    return rir_type_equal(a, b);
}

size_t rir_type_bytesize(const struct rir_type *t)
{
    if (t->category == RIR_TYPE_ELEMENTARY) {
        return elementary_type_to_bytesize(t->etype);
    }
    return rir_typedef_bytesize(t->tdef);
}

const struct RFstring *rir_type_string(const struct rir_type *t)
{
    if (t->category == RIR_TYPE_ELEMENTARY) {
        return RFS(RF_STR_PF_FMT"%s",
                   RF_STR_PF_ARG(type_elementary_get_str(t->etype)),
                   t->is_pointer ? "*" : "");
    } else {
        return RFS(RF_STR_PF_FMT"%s",
                   RF_STR_PF_ARG((t->tdef->name)),
                   t->is_pointer ? "*" : "");
    }
}

const struct rir_type *rir_type_comp_member_type(const struct rir_type *t, uint32_t idx)
{
    RF_ASSERT(rir_type_is_composite(t), "Expected composite type");
    return rir_typedef_typeat(t->tdef, idx);
}

int rir_type_union_matched_type_from_fncall(const struct rir_type *t, const struct ast_node *n, struct rir_ctx *ctx)
{
    RF_ASSERT(rir_type_is_union(t), "Expected a union type");
    const struct type *matched = ast_fncall_params_type(n);
    struct rir_type_arr tarr;
    if (!rir_typearr_from_type(&tarr, matched, ARGARR_AT_TYPEDESC, ctx)) {
        return -1;
    }

    int index = 0;
    struct rir_type **argtype;
    darray_foreach(argtype, t->tdef->argument_types) {
        //check if this is the type that matches the function call
        if ((*argtype)->category == RIR_TYPE_ELEMENTARY) {
            if (darray_size(tarr) == 1 && rir_type_equal(*argtype, darray_item(tarr, 0))) {
                goto end;
            }
        } else { // composite
            if (rir_typearr_equal(&tarr, &(*argtype)->tdef->argument_types)) {
                goto end;
            }
        }
        ++index;
    }
    // failure
    index = -1;

end:
    rir_typearr_deinit(&tarr, ctx->rir);
    return index;
}
