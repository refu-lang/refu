#include <ir/rir_utils.h>
#include <ir/rir.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>
#include <ir/rir_expression.h>
#include <ir/rir_object.h>

#include <Utils/log.h>

struct rir_value g_rir_const_1;
struct rir_value g_rir_const_m1;
static bool utils_created = false;

bool rir_utils_create()
{
    if (!rir_constantval_init_fromint32(&g_rir_const_1, 1)) {
        return false;
    }
    if (!rir_constantval_init_fromint32(&g_rir_const_m1, -1)) {
        return false;
    }
    utils_created = true;
    return true;
}

void rir_utils_destroy()
{
    if (utils_created) {
        rir_value_deinit(&g_rir_const_m1);
        rir_value_deinit(&g_rir_const_1);
    }
    utils_created = false;
}

struct rir_value *rir_getread_val(struct rir_expression *e, struct rir_ctx *ctx)
{
    struct rir_expression *ret = rir_getread_expr(e, ctx);
    return ret ? &ret->val : NULL;
}

struct rir_expression *rir_getread_expr(struct rir_expression *e, struct rir_ctx *ctx)
{
    struct rir_expression *ret = NULL;
    // gotta read the memory value from an alloca
    // unless it's a string, which is passed by pointer at least at the moment
    if (e->type == RIR_EXPRESSION_ALLOCA &&
        !rir_ltype_is_specific_elementary(e->alloca.type, ELEMENTARY_TYPE_STRING)
    ) {
        struct rir_expression *read;
        read = rir_read_create(&e->val, ctx);
        if (!read) {
            RF_ERROR("Failed to create a read RIR instruction");
            return NULL;
        }
        rirctx_block_add(ctx, read);
        ret = read;
    } else {
        ret = e;
    }
    return ret;
}

struct rir_object *rir_getread_obj(struct rir_object *o, struct rir_ctx *ctx)
{
    struct rir_object *ret = NULL;
    RF_ASSERT(o->category == RIR_OBJ_EXPRESSION, "Expected an expression object");
    struct rir_expression *e = &o->expr;
    // gotta read the memory value from an alloca
    // unless it's a string, which is passed by pointer at least at the moment
    if (e->type == RIR_EXPRESSION_ALLOCA &&
        !rir_ltype_is_specific_elementary(e->alloca.type, ELEMENTARY_TYPE_STRING)
    ) {
        struct rir_object *read;
        read = rir_read_create_obj(&e->val, ctx);
        if (!read) {
            RF_ERROR("Failed to create a read RIR instruction");
            return NULL;
        }
        rirctx_block_add(ctx, &read->expr);
        ret = read;
    } else {
        ret = o;
    }
    return ret;
}
