#include <ir/rir_constant.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/constants.h>

struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->constant = c->constant;
    rir_expression_init(ret, RIR_EXPRESSION_CONSTANT, ctx);
    return ret;
}

struct rir_value *rir_constantval_fromint(int64_t n)
{
    struct rir_value *ret;
    struct ast_constant c;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->category = RIR_VALUE_CONSTANT;
    ast_constant_init_int(&c, n);
    return rir_value_constant_init(ret, &c) ? ret : NULL;
}

bool rir_constant_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RF_ASSERT(e->type == RIR_EXPRESSION_CONSTANT, "Expected constant");
    switch (e->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%" PRId64, e->constant.value.integer));
        break;
    case CONSTANT_NUMBER_FLOAT:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%f", e->constant.value.floating));
        break;
    case CONSTANT_BOOLEAN:
        ret = rf_stringx_append(ctx->rir->buff, RFS("%s", e->constant.value.boolean ? "true" : "false"));
        break;
    }
    return ret;
}
