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
        break;
    case RIR_OBJ_ARGUMENT:
        break;
    case RIR_OBJ_BLOCK:
        break;
    case RIR_OBJ_TYPEDEF:
        rir_typedef_deinit(&obj->tdef);
        break;
    }
    free(obj);
}

struct rir_value *rir_object_value(struct rir_object *obj)
{
    switch (obj->category) {
    case RIR_OBJ_EXPRESSION:
        return &obj->expr.val;
    case RIR_OBJ_ARGUMENT:
        return &obj->arg.val;
    default:
        RF_ASSERT_OR_EXIT(false, "Unexpected rir object value");
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
