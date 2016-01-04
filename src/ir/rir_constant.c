#include <ir/rir_constant.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/constants.h>

struct rir_object *rir_constant_create_obj(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, rir_ctx_rir(ctx));
    if (!ret) {
        return NULL;
    }
    ret->expr.type = RIR_EXPRESSION_CONSTANT;
    return rir_value_constant_init(&ret->expr.val, &c->constant, ELEMENTARY_TYPE_TYPES_COUNT) ? ret : NULL;
}

struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_constant_create_obj(c, ctx);
    return obj ? &obj->expr : NULL;
}

struct rir_value *rir_constantval_create_fromint64(int64_t n, struct rir *r)
{
    struct rir_value *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_constantval_init_fromint64(ret, n)) {
        free(ret);
        ret = NULL;
    }
    // note that this value will need destruction
    rir_freevalues_add(r, ret);
    return ret;
}

bool rir_constantval_init_fromint64(struct rir_value *v, int64_t n)
{
    struct ast_constant c;
    v->category = RIR_VALUE_CONSTANT;
    ast_constant_init_int(&c, n);
    return rir_value_constant_init(v, &c, ELEMENTARY_TYPE_INT_64);
}

struct rir_value *rir_constantval_create_fromint32(int32_t n, struct rir *r)
{
    struct rir_value *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_constantval_init_fromint32(ret, n)) {
        free(ret);
        ret = NULL;
    }
    // note that this value will need destruction
    rir_freevalues_add(r, ret);
    return ret;
}

bool rir_constantval_init_fromint32(struct rir_value *v, int32_t n)
{
    struct ast_constant c;
    v->category = RIR_VALUE_CONSTANT;
    ast_constant_init_int(&c, n);
    return rir_value_constant_init(v, &c, ELEMENTARY_TYPE_INT_32);
}

const struct RFstring *rir_constant_string(const struct rir_value *val)
{
    RF_ASSERT(val->category == RIR_VALUE_CONSTANT, "Expected constant");
    switch (val->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        return RFS("%" PRId64,val->constant.value.integer);
    case CONSTANT_NUMBER_FLOAT:
        return RFS("%.4f", val->constant.value.floating);
    case CONSTANT_BOOLEAN:
        return RFS("%s", val->constant.value.boolean ? "true" : "false");
    }
    RF_CRITICAL_FAIL("Illegal constan type. Should never happen");
    return NULL;
}

bool rir_constant_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret;
    RFS_PUSH();
    ret = rf_stringx_append(ctx->rir->buff, rir_constant_string(&e->val));
    RFS_POP();
    return ret;
}
