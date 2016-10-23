#include <ir/rir_variable.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

static bool rir_variable_init(
    struct rir_object *obj,
    struct rir_type *type,
    enum rir_pos pos,
    rir_data data
)
{
    return rir_value_variable_init(&obj->variable.val, obj, type, pos, data);
}

struct rir_object *rir_variable_create(
    struct rir_type *type,
    enum rir_pos pos,
    rir_data data)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_VARIABLE, rir_data_rir(data));
    if (!ret) {
        return NULL;
    }
    if (!rir_variable_init(ret, type, pos, data)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_variable_deinit(struct rir_variable *var)
{
    rir_value_deinit(&var->val);
}

i_INLINE_INS struct rir_type *rir_variable_type(struct rir_variable *v);
