#include <ir/rir_expression.h>
#include <ir/rir.h>
#include <ir/rir_value.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_function.h>
#include <ir/rir_argument.h>
#include <ir/rir_constant.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


bool rir_expression_init(struct rir_expression *expr,
                         enum rir_expression_type type,
                         struct rir_ctx *ctx)
{
    expr->type = type;
    switch (expr->type) {
    case RIR_EXPRESSION_CONSTANT:
        if (!rir_value_constant_init(&expr->val, &expr->constant)) {
            return false;
        }
        break;
    case RIR_EXPRESSION_WRITE:
    case RIR_EXPRESSION_RETURN:
        rir_value_nil_init(&expr->val);
        break;
    default:
        if (!rir_value_variable_init(&expr->val, expr, ctx)) {
            return false;
        }
    }
    return true;
}


static void rir_expression_deinit(struct rir_expression *expr)
{
    // TODO
}

void rir_expression_destroy(struct rir_expression *expr)
{
    rir_expression_deinit(expr);
    // return expresssions get initialized on the stack so no need to free
    if (expr->type != RIR_EXPRESSION_RETURN) {
        free(expr);
    }
}

static inline void rir_alloca_init(struct rir_alloca *obj,
                                   const struct rir_ltype *type,
                                   uint64_t num)
{
    obj->type = type;
    obj->num = num;
}

struct rir_expression *rir_alloca_create(const struct rir_ltype *type,
                                         uint64_t num,
                                         struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_alloca_init(&ret->alloca, type, num);
    rir_expression_init(ret, RIR_EXPRESSION_ALLOCA, ctx);
    return ret;
}

static inline void rir_alloca_deinit(struct rir_expression *obj)
{
    return;// TODO
}

bool rir_return_init(struct rir_expression *ret,
                     const struct rir_expression *val,
                     struct rir_ctx *ctx)
{
    ret->ret.val = val;
    if (!rir_expression_init(ret, RIR_EXPRESSION_RETURN, ctx)) {
        return false;
    }
    return true;
}

struct rir_expression *rir_return_create(const struct rir_expression *val, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_return_init(ret, val, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

bool rir_expression_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();
    switch(e->type) {
    case RIR_EXPRESSION_FNCALL:
        if (!rf_stringx_append(ctx->rir->buff, RFS("fncall"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ALLOCA:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RITOSTR_INDENT RF_STR_PF_FMT" = alloca(" RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_ltype_string(e->alloca.type)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_READ:
        if (!rf_stringx_append_cstr(ctx->rir->buff, "read()")) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_CONSTRUCT:
        if (!rf_stringx_append(ctx->rir->buff, RFS("construct"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_CONSTANT:
        if (!rir_constant_tostring(ctx, e)) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ADD:
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
    case RIR_EXPRESSION_CMP:
    case RIR_EXPRESSION_WRITE:
    case RIR_EXPRESSION_OBJMEMBERAT:
        if (!rir_binaryop_tostring(ctx, e)) {
            goto end;
        }
        break;
    // PLACEHOLDER, should not make it into actual production
    case RIR_EXPRESSION_LOGIC_AND:
    case RIR_EXPRESSION_LOGIC_OR:
    case RIR_EXPRESSION_RETURN:
    case RIR_EXPRESSION_PLACEHOLDER:
        if (!rf_stringx_append(ctx->rir->buff, RFS("NOT_IMPLEMENTED\n"))) {
            goto end;
        }
        break;
    }
    ret = true;
end:
    RFS_POP();
    return ret;
}
