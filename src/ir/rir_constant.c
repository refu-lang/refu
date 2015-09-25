#include <ir/rir_constant.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/constants.h>

struct rir_object *rir_constant_create_obj(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.type = RIR_EXPRESSION_CONSTANT;
    return rir_value_constant_init(&ret->expr.val, &c->constant) ? ret : NULL;
}

struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_constant_create_obj(c, ctx);
    return obj ? &obj->expr : NULL;
}

struct rir_value *rir_constantval_create_fromint(int64_t n)
{
    struct rir_value *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_constantval_init_fromint(ret, n)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

bool rir_constantval_init_fromint(struct rir_value *v, int64_t n)
{
    struct ast_constant c;
    v->category = RIR_VALUE_CONSTANT;
    ast_constant_init_int(&c, n);
    return rir_value_constant_init(v, &c);
}

bool rir_constant_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RF_ASSERT(e->type == RIR_EXPRESSION_CONSTANT, "Expected constant");
    switch (e->val.constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%" PRId64, e->val.constant.value.integer));
        break;
    case CONSTANT_NUMBER_FLOAT:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%f", e->val.constant.value.floating));
        break;
    case CONSTANT_BOOLEAN:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%s", e->val.constant.value.boolean ? "true" : "false"));
        break;
    }
    return ret;
}
