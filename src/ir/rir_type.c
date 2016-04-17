#include <ir/rir_type.h>

#include <rflib/utils/fixed_memory_pool.h>

#include <ir/rir.h>
#include <ir/rir_typedef.h>
#include <ir/rir_object.h>
#include <types/type.h>
#include <ast/function.h>
#include <analyzer/type_set.h>
#include <utils/common_strings.h>

i_INLINE_INS bool rir_type_is_elementary(const struct rir_type *t);
i_INLINE_INS bool rir_type_is_specific_elementary(const struct rir_type *t,
                                                  enum elementary_type etype);
i_INLINE_INS bool rir_type_is_composite(const struct rir_type *t);
i_INLINE_INS int64_t rir_type_array_size(const struct rir_type *t);
bool rir_type_is_union(const struct rir_type *t)
{
    return t->category == RIR_TYPE_COMPOSITE && t->tdef->is_union;
}

void rir_type_elem_init(struct rir_type *t, enum elementary_type etype, bool is_pointer)
{
    t->category = RIR_TYPE_ELEMENTARY;
    t->etype = etype;
    t->is_pointer = is_pointer;
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

struct rir_type *rir_type_elem_get_from_string(
    struct rir *r,
    const struct RFstring *name,
    bool is_pointer)
{
    enum elementary_type etype = type_elementary_from_str(name);
    return etype == ELEMENTARY_TYPE_TYPES_COUNT
        ? NULL
        : rir_type_elem_get_or_create(r, etype, is_pointer);
}

static inline const struct RFstring *rir_form_elemtype_string(
    enum elementary_type etype,
    bool is_pointer)
{
    return RFS(
        RFS_PF"%s",
        RFS_PA(type_elementary_get_str(etype)),
        is_pointer ? "*" : ""
    );
}

static inline const struct RFstring *rir_form_comptype_string(
    const struct rir_typedef *tdef,
    bool is_pointer)
{
    return RFS(RFS_PF"%s", RFS_PA((&tdef->name)), is_pointer ? "*" : "");
}

static inline const struct RFstring *rir_form_arrtype_string(
    const struct rir_type *pointing_type,
    int64_t dimensions,
    bool is_pointer)
{
    return dimensions <= 0
        ? RFS("["RFS_PF"]%s", RFS_PA(rir_type_string(pointing_type)), is_pointer ? "*" : "")
        : RFS("[%"PRId64"x"RFS_PF"]%s", dimensions, RFS_PA(rir_type_string(pointing_type)), is_pointer ? "*" : "");
}

struct rir_type *rir_type_elem_get_or_create(
    struct rir *r,
    enum elementary_type etype,
    bool is_pointer)
{
    RFS_PUSH();
    const struct RFstring *id = rir_form_elemtype_string(etype, is_pointer);
    struct rir_type *ret = rirtype_strmap_get(&r->types_map, id);
    if (ret) {
        goto end;
    }

    // else
    if (!(ret = rir_type_alloc(r))) {
        goto end;
    }
    rir_type_elem_init(ret, etype, is_pointer);
    rirtype_strmap_add(&r->types_map, id, ret);

end:
    RFS_POP();
    return ret;
}

void rir_type_comp_init(struct rir_type *t, const struct rir_typedef *def, bool is_pointer)
{
    t->category = RIR_TYPE_COMPOSITE;
    t->tdef = def;
    t->is_pointer = is_pointer;
}

struct rir_type *rir_type_comp_get_or_create(
    const struct rir_typedef *def,
    struct rir *r,
    bool is_pointer)
{
    RFS_PUSH();
    const struct RFstring *id = rir_form_comptype_string(def, is_pointer);
    struct rir_type *ret = rirtype_strmap_get(&r->types_map, id);
    if (ret) {
        RF_ASSERT(ret->is_pointer == is_pointer, "Should never happen.");
        goto end;
    }

    // else
    if (!(ret = rir_type_alloc(r))) {
        goto end;
    }
    rir_type_comp_init(ret, def, is_pointer);
    rirtype_strmap_add(&r->types_map, id, ret);
end:
    RFS_POP();
    return ret;
}

void rir_type_arr_init(
    struct rir_type *t,
    const struct rir_type *pointing_type,
    int64_t size,
    bool is_pointer)
{
    t->category = RIR_TYPE_ARRAY;
    t->is_pointer = is_pointer;
    t->array.type = pointing_type;
    t->array.size = size;
}

struct rir_type *rir_type_arr_get_or_create(
    struct rir *r,
    const struct rir_type *pointing_type,
    int64_t size,
    bool is_pointer)
{
    RFS_PUSH();
    const struct RFstring *id = rir_form_arrtype_string(
        pointing_type,
        size,
        is_pointer
    );
    struct rir_type *ret = rirtype_strmap_get(&r->types_map, id);
    if (ret) {
        RF_ASSERT(ret->is_pointer == is_pointer, "Should never happen.");
        goto end;
    }

    // else
    if (!(ret = rir_type_alloc(r))) {
        goto end;
    }
    rir_type_arr_init(ret, pointing_type, size, is_pointer);
    rirtype_strmap_add(&r->types_map, id, ret);
end:
    RFS_POP();
    return ret;
}

struct rir_type *rir_type_create_from_type(const struct type *t, struct rir_ctx *ctx)
{
    struct rir_typedef *tdef;
    struct rir_type *ret = NULL;
    if (t->category == TYPE_CATEGORY_ELEMENTARY) {
        ret = rir_type_elem_get_or_create(rir_ctx_rir(ctx), t->elementary.etype, false);
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
        ret = rir_type_comp_get_or_create(tdef, rir_ctx_rir(ctx), false);
    } else if (t->category == TYPE_CATEGORY_OPERATOR) {
        struct rir_object *obj = rir_ctx_st_getobj(ctx, type_get_unique_type_str(t));
        if (!obj) {
            symbol_table_print(rir_ctx_curr_st(ctx));
            type_objset_print(ctx->common.rir->types_set);
            RF_ERROR("Could not find operator type, equivalent typedef RIR object in symbol table");
            return NULL;
        }
        tdef = rir_object_get_typedef(obj);
        if (!tdef) {
            RF_ERROR("Could not retrieve typedef from rir object. Invalid rir object?");
            return NULL;
        }
        ret = rir_type_comp_get_or_create(tdef, ctx->common.rir, false);
    } else {
        RF_CRITICAL_FAIL("Unexpected type category");
    }

    if (t->array) {
        // for now totally ignore multi-dimension arrays when creating rir type arrays
        ret = rir_type_arr_get_or_create(
            rir_ctx_rir(ctx),
            ret,
            darray_size(t->array->dimensions) == 0 ? -1 : darray_item(t->array->dimensions, 0),
            false // not a pointer to array
        );
    }
    return ret;
}

struct rir_type *rir_type_get_or_create_from_other(
    const struct rir_type *other,
    struct rir *r,
    bool is_pointer)
{
    switch (other->category) {
    case RIR_TYPE_ELEMENTARY:
        return rir_type_elem_get_or_create(r, other->etype, is_pointer);
    case RIR_TYPE_COMPOSITE:
        return rir_type_comp_get_or_create(other->tdef, r, is_pointer);
    case RIR_TYPE_ARRAY:
        return rir_type_arr_get_or_create(
            r,
            other->array.type,
            other->array.size,
            is_pointer
        );
    default:
        RF_ASSERT_OR_CRITICAL(false, return NULL, "Unexpected type category");
        break;
    }
}

bool rir_type_equal(const struct rir_type *a, const struct rir_type *b)
{
    if (a->category != b->category) {
        return false;
    }
    switch (a->category) {
    case RIR_TYPE_ELEMENTARY:
        if (a->etype != b->etype) {
            return false;
        }
        break;

    case RIR_TYPE_COMPOSITE:
        if (!rir_typedef_equal(a->tdef, b->tdef)) {
            return false;
        }
        break;
    case RIR_TYPE_ARRAY:
        if (!rir_type_equal(a->array.type, b->array.type)) {
            return false;
        }
        if (a->array.size != b->array.size) {
            return false;
        }
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Unexpected type category");
        break;
    }
    return true;
}

bool rir_type_identical(const struct rir_type *a, const struct rir_type *b)
{
    if (!a || !b) { // if one is NULL, the other one has to be too
        return a == b;
    }
    if (a->is_pointer != b->is_pointer) {
        return false;
    }
    return rir_type_equal(a, b);
}

size_t rir_type_bytesize(const struct rir_type *t)
{
    switch (t->category) {
    case RIR_TYPE_ELEMENTARY:
        return elementary_type_to_bytesize(t->etype);
    case RIR_TYPE_COMPOSITE:
        return rir_typedef_bytesize(t->tdef);
    case RIR_TYPE_ARRAY:
        return rir_type_bytesize(t->array.type) * t->array.size;
    }
    RF_ASSERT_OR_CRITICAL(false, return 0, "Unexpected type category");
}

const struct RFstring *rir_type_string(const struct rir_type *t)
{
    switch (t->category) {
    case RIR_TYPE_ELEMENTARY:
        return rir_form_elemtype_string(t->etype, t->is_pointer);
    case RIR_TYPE_COMPOSITE:
        return rir_form_comptype_string(t->tdef, t->is_pointer);
    case RIR_TYPE_ARRAY:
        return rir_form_arrtype_string(t->array.type, t->array.size, t->is_pointer);
    }
    RF_ASSERT_OR_CRITICAL(false, return NULL, "Unexpected type category");
}

const struct RFstring *rir_type_category_str(const struct rir_type *t)
{
    switch (t->category) {
    case RIR_TYPE_ELEMENTARY:
        return &g_str_elementary;
    case RIR_TYPE_COMPOSITE:
        return &g_str_composite;
    case RIR_TYPE_ARRAY:
        return &g_str_array;
    }
    RF_ASSERT_OR_CRITICAL(false, return NULL, "Unexpected type category");
}

const struct rir_type *rir_type_comp_member_type(const struct rir_type *t, uint32_t idx)
{
    RF_ASSERT(rir_type_is_composite(t), "Expected composite type");
    return rir_typedef_typeat(t->tdef, idx);
}

int rir_type_union_matched_type_from_fncall(
    const struct rir_type *t,
    const struct ast_node *n,
    struct rir_ctx *ctx)
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
        switch ((*argtype)->category) {
        case RIR_TYPE_ELEMENTARY:
            if (darray_size(tarr) == 1 && rir_type_equal(*argtype, darray_item(tarr, 0))) {
                goto end;
            }
            break;
        case RIR_TYPE_COMPOSITE:
            if (rir_typearr_equal(&tarr, &(*argtype)->tdef->argument_types)) {
                goto end;
            }
            break;
        case RIR_TYPE_ARRAY:
            RF_ASSERT(false, "TODO");
            break;
        }
        ++index;
    }
    // failure
    index = -1;

end:
    rir_typearr_deinit(&tarr, ctx->common.rir);
    return index;
}
