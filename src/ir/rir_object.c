#include <ir/rir_object.h>

#include <rfbase/utils/memory.h>
#include <rfbase/string/core.h>

#include <ir/rir.h>
#include <ir/rir_function.h>

struct rir_object *rir_object_create(enum rir_obj_category category, struct rir *r)
{
    struct rir_object *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    RF_STRUCT_ZERO(ret);
    ret->category = category;
    rf_ilist_add(&r->objects,  &ret->ln);
    return ret;
}

void rir_object_destroy(struct rir_object *obj)
{
    switch(obj->category) {
    case RIR_OBJ_EXPRESSION:
        rir_expression_deinit(&obj->expr);
        break;
    case RIR_OBJ_BLOCK:
        rir_block_deinit(&obj->block);
        break;
    case RIR_OBJ_TYPEDEF:
        rir_typedef_deinit(&obj->tdef);
        break;
    case RIR_OBJ_GLOBAL:
        rir_global_deinit(&obj->global);
        break;
    case RIR_OBJ_VARIABLE:
        rir_variable_deinit(&obj->variable);
        break;
    }
    free(obj);
}

struct rir_value *rir_object_value(struct rir_object *obj)
{
    switch (obj->category) {
    case RIR_OBJ_EXPRESSION:
        return &obj->expr.val;
    case RIR_OBJ_GLOBAL:
        return &obj->global.val;
    case RIR_OBJ_VARIABLE:
        return &obj->variable.val;
    case RIR_OBJ_BLOCK:
        return &obj->block.label;
    default:
        RF_CRITICAL_FAIL("Unexpected rir object value");
        break;
    }
    return NULL;
}

void rir_object_listrem(struct rir_object *obj, struct rir *r, struct rir_fndef *current_fn)
{
    rf_ilist_delete_from(&r->objects, &obj->ln);
    // also remove it from any string maps it can be found in
    struct rir_value *val = rir_object_value(obj);
    struct rirobj_strmap *map = current_fn ? &current_fn->map : &r->map;
    struct RFstring *str = strmap_del(map, &val->id, NULL);
    if (!str) {
        str = strmap_del(&r->map, &val->id, NULL);
    }
    RF_ASSERT(str, "Could not find object for removal in the current function or in the global rir string map");
}

void rir_object_listrem_destroy(struct rir_object *obj, struct rir *r, struct rir_fndef *current_fn)
{
    rir_object_listrem(obj, r, current_fn);
    rir_object_destroy(obj);
}

struct rir_typedef *rir_object_get_typedef(struct rir_object *obj)
{
    if (obj->category == RIR_OBJ_TYPEDEF) {
        return &obj->tdef;
    } else if (obj->category == RIR_OBJ_VARIABLE) {
        struct rir_type *vtype = rir_variable_type(&obj->variable);
        if (!rir_type_is_composite(vtype)) {
            return NULL;
        }
        return (struct rir_typedef*)vtype->tdef;
    }
    return NULL;
}

static const struct RFstring rir_obj_category_strings[] = {
    [RIR_OBJ_EXPRESSION] = RF_STRING_STATIC_INIT("RIR expression object"),
    [RIR_OBJ_BLOCK] = RF_STRING_STATIC_INIT("RIR block object"),
    [RIR_OBJ_TYPEDEF] = RF_STRING_STATIC_INIT("RIR typedef object"),
    [RIR_OBJ_GLOBAL] = RF_STRING_STATIC_INIT("RIR global object"),
    [RIR_OBJ_VARIABLE] = RF_STRING_STATIC_INIT("RIR variable object")
};

const struct RFstring *rir_object_category_str(const struct rir_object *obj)
{
    return &rir_obj_category_strings[obj->category];
}

const struct RFstring *rir_object_string(const struct rir_object *obj)
{
    if (obj->category == RIR_OBJ_EXPRESSION) {
        return rir_expression_type_string(&obj->expr);
    }
    return rir_object_category_str(obj);
}

i_INLINE_INS struct rir_expression *rir_object_to_expr(struct rir_object *obj);
i_INLINE_INS struct rir_value *rir_object_block_label(struct rir_object *obj);
