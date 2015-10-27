#include <ir/rir_variable.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

static bool rir_variable_init(struct rir_object *obj,
                              struct rir_ltype *type,
                              struct rir_ctx  *ctx)
{
    return rir_value_variable_init(&obj->variable.val, obj, type, ctx);
}

struct rir_object *rir_variable_create(struct rir_ltype *type,
                                       struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_VARIABLE, ctx->rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_variable_init(ret, type, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_variable_deinit(struct rir_variable *var)
{
    rir_value_deinit(&var->val);
}

i_INLINE_INS struct rir_ltype *rir_variable_type(struct rir_variable *v);
