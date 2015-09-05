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
