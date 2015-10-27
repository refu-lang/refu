#include <ir/rir_object.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <Utils/memory.h>

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
    default:
        RF_CRITICAL_FAIL("Unexpected rir object value");
        break;
    }
    return NULL;
}

void rir_object_listrem(struct rir_object *obj, struct rir_ctx *ctx)
{
    rf_ilist_delete_from(&ctx->rir->objects, &obj->ln);
    // also remove it from any string maps it can be found in
    struct rir_value *val = rir_object_value(obj);
    struct rirobj_strmap *map = ctx->current_fn ? &ctx->current_fn->map : &ctx->rir->map;
    struct RFstring *str = strmap_del(map, &val->id, NULL);
    if (!str) {
        str = strmap_del(&ctx->rir->map, &val->id, NULL);
    }
    RF_ASSERT(str, "Could not find object for removal in the current function or in the global rir string map");
}

void rir_object_listrem_destroy(struct rir_object *obj, struct rir_ctx *ctx)
{
    rir_object_listrem(obj, ctx);
    rir_object_destroy(obj);
}

struct rir_typedef *rir_object_get_typedef(struct rir_object *obj)
{
    if (obj->category == RIR_OBJ_TYPEDEF) {
        return &obj->tdef;
    } else if (obj->category == RIR_OBJ_VARIABLE) {
        struct rir_ltype *vtype = rir_variable_type(&obj->variable);
        if (!rir_ltype_is_composite(vtype)) {
            return NULL;
        }
        return (struct rir_typedef*)vtype->tdef;
    }
    return NULL;
}
