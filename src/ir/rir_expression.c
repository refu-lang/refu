#include <ir/rir_expression.h>
#include <ir/rir.h>
#include <ir/rir_value.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_argument.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


bool rir_expression_init(struct rir_expression *expr,
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
    case RIR_EXPRESSION_LABEL:
        if (!rir_value_init(&expr->val, RIR_VALUE_LABEL, expr, ctx)) {
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
    // return expresssions get initialized on the stack so no need to free
    if (expr->type != RIR_EXPRESSION_RETURN) {
        free(expr);
    }
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

bool rir_return_init(struct rir_expression *ret,
                     const struct rir_expression *val,
                     struct rir_ctx *ctx)
{
    if (!rir_expression_init(ret, RIR_EXPRESSION_RETURN, ctx)) {
        return false;
    }
    ret->ret.val = val;
    return true;
}

struct rir_expression *rir_label_create(const struct rir_block *b, unsigned index, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, RIR_EXPRESSION_LABEL, ctx)) {
        free(ret);
        ret = NULL;
    }
    ret->label.block = b;
    ret->label.index = index;
    return ret;
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
    case RIR_EXPRESSION_LABEL:
        if (!rf_stringx_append(r->buff, RFS(RF_STR_PF_FMT":\n",
                                            RF_STR_PF_ARG(rir_value_string(&e->val))))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_FNCALL:
        if (!rf_stringx_append(r->buff, RFS("fncall"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ALLOCA:
        if (!rf_stringx_append(
                r->buff,
                RFS(RF_STR_PF_FMT" = alloca(" RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_ltype_string(e->alloca.type)))
            )) {
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
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
    case RIR_EXPRESSION_CMP:
    case RIR_EXPRESSION_WRITE:
        if (!rir_binaryop_tostring(r, e)) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_LOGIC_AND:
        if (!rf_stringx_append(r->buff, RFS("logic_and"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_LOGIC_OR:
        if (!rf_stringx_append(r->buff, RFS("logic_or"))) {
            goto end;
        }
        break;
    // PLACEHOLDER, should not make it into actual production
    case RIR_EXPRESSION_READ:
    case RIR_EXPRESSION_PLACEHOLDER:
        if (!rf_stringx_append(r->buff, RFS("NOT_IMPLEMENTED\n"))) {
            goto end;
        }
        break;
    }
    ret = true;
end:
    RFS_POP();
    return ret;
}
