#include <ir/rir_convert.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir.h>


struct rir_object *rir_convert_create_obj(const struct rir_value *convval,
                                          const struct rir_ltype *totype,
                                          struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.convert.val = convval;
    ret->expr.convert.type = totype;
    if (!rir_expression_init(ret, RIR_EXPRESSION_CONVERT, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_convert_create(const struct rir_value *convval,
                                          const struct rir_ltype *totype,
                                          struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_convert_create_obj(convval, totype, ctx);
    return obj ? &obj->expr : NULL;
}

const struct rir_value *rir_maybe_convert(const struct rir_value *val,
                                          const struct rir_ltype *checktype,
                                          struct rir_ctx *ctx)
{
    if (!rir_ltype_equal(val->type, checktype)) {
        struct rir_expression *e;
        if (!(e = rir_convert_create(val, rir_ltype_create_from_other(checktype, false), ctx))) {
            return NULL;;
        }
        rirctx_block_add(ctx, e);
        val = &e->val;
    }
    return val;
}
