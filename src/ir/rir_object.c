#include <ir/rir_object.h>
#include <ir/rir.h>
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

void rir_object_listrem(struct rir_object *obj, struct rir *r)
{
    rf_ilist_delete_from(&r->objects, &obj->ln);
    // also remove it from any string map ... currently will only work for the global map
    // if this function is called and it's not found in the global map it's either
    // in one of the function maps or nowhere. Should not happen, so assert
    struct rir_value *val = rir_object_value(obj);
    struct RFstring *str = strmap_del(&r->map, &val->id, NULL);
    RF_ASSERT(str, "Could not find object for removal in the global rir string map");
}

void rir_object_listrem_destroy(struct rir_object *obj, struct rir *r)
{
    rf_ilist_delete_from(&r->objects, &obj->ln);
    rir_object_destroy(obj);
}
