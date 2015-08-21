#include <ir/rir_expression.h>
#include <ir/rir.h>
#include <ir/rir_value.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


static inline bool rir_expression_init(struct rir_expression *expr,
                                       enum rir_expression_type type,
                                       struct rir_ctx *ctx)
{
    expr->type = type;
    switch (expr->type) {
    case RIR_EXPRESSION_CONSTANT:
        if (!rir_value_init(&expr->val, RIR_VALUE_CONSTANT, expr, ctx)) {
            return false;
        }
        break;
    default:
        if (!rir_value_init(&expr->val, RIR_VALUE_VARIABLE, expr, ctx)) {
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
    free(expr);
}



static inline void rir_binaryop_init(struct rir_binaryop *op,
                                     const struct rir_value *a,
                                     const struct rir_value *b)
{
    op->a = a;
    op->b = b;
}

struct rir_expression *rir_binaryop_create(enum rir_expression_type type,
                                           const struct rir_value *a,
                                           const struct rir_value *b,
                                           struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, type, ctx)) {
        free(ret);
        ret = NULL;
        goto end;
    }
    rir_binaryop_init(&ret->binaryop, a, b);
end:
    return ret;
}

static inline bool rir_alloca_init(struct rir_alloca *obj,
                                  const struct rir_ltype *type,
                                  uint64_t num)
{
    obj->type = type;
    obj->num = num;
    return true;
}

struct rir_expression *rir_alloca_create(const struct rir_ltype *type,
                                         uint64_t num,
                                         struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, RIR_EXPRESSION_ALLOCA, ctx);
    if (!rir_alloca_init(&ret->alloca, type, num)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static inline void rir_alloca_deinit(struct rir_expression *obj)
{
    return;// TODO
}

struct rir_expression *rir_return_create(const struct rir_expression *val, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, RIR_EXPRESSION_RETURN, ctx);
    ret->ret.val = val;
    return ret;
}

struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->constant = c->constant;
    rir_expression_init(ret, RIR_EXPRESSION_CONSTANT, ctx);
    return ret;
}

bool rir_constant_tostring(struct rir *r, const struct rir_expression *e)
{
    bool ret = false;
    RF_ASSERT(e->type == RIR_EXPRESSION_CONSTANT, "Expected constant");
    switch (e->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        ret = rf_stringx_append(r->buff, RFS("%" PRId64, e->constant.value.integer));
        break;
    case CONSTANT_NUMBER_FLOAT:
        ret = rf_stringx_append(r->buff, RFS("%d", e->constant.value.floating));
        break;
    case CONSTANT_BOOLEAN:
        ret = rf_stringx_append(r->buff, RFS("%s", e->constant.value.boolean ? "true" : "false"));
        break;
    }
    return ret;
}

bool rir_expression_tostring(struct rir *r, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();
    switch(e->type) {
    case RIR_EXPRESSION_FNCALL:
        if (!rf_stringx_append(r->buff, RFS("fncall"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ALLOCA:
        if (!rf_stringx_append(r->buff, RFS("alloca"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_CONSTRUCT:
        if (!rf_stringx_append(r->buff, RFS("construct"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_RETURN:
        if (!rf_stringx_append(
                r->buff,
                RFS("return("RF_STR_PF_FMT")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->ret.val->val)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_CONSTANT:
        if (!rir_constant_tostring(r, e)) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ADD:
        if (!rf_stringx_append(
                r->buff,
                RFS(RF_STR_PF_FMT" = add(" RF_STR_PF_FMT ", " RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.a)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.b)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_SUB:
        if (!rf_stringx_append(r->buff, RFS("sub"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_MUL:
        if (!rf_stringx_append(r->buff, RFS("mul"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_DIV:
        if (!rf_stringx_append(r->buff, RFS("div"))) {
            goto end;
        }
        break;
    // PLACEHOLDER, should not make it into actual production
    case RIR_EXPRESSION_PLACEHOLDER:
        if (!rf_stringx_append(r->buff, RFS("NOT_IMPLEMENTED\n"))) {
            goto end;
        }
        break;
    }

    /* if (!rf_stringx_append(r->buff, RFS("\n"))) { */
    /*     goto end; */
    /* } */
    //success
    ret = true;
end:
    RFS_POP();
    return ret;
}
