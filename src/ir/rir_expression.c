#include <ir/rir_expression.h>
#include <ir/rir.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


static inline void rir_expression_init(struct rir_expression *expr, enum rir_expression_type type)
{
    expr->type = type;
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
                                     const struct rir_expression *a,
                                     const struct rir_expression *b)
{
    op->a = a;
    op->b = b;
}

struct rir_expression *rir_binaryop_create(enum rir_expression_type type,
                                           const struct rir_expression *a,
                                           const struct rir_expression *b)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, type);
    rir_binaryop_init(&ret->binaryop, a, b);
    return ret;
}

static inline bool rir_alloca_init(struct rir_alloca *obj,
                                  const struct rir_type *type,
                                  uint64_t num)
{
    obj->type = type;
    obj->num = num;
    return true;
}

struct rir_expression *rir_alloca_create(const struct rir_type *type, uint64_t num)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, RIR_EXPRESSION_ALLOCA);
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

struct rir_expression *rir_return_create(const struct rir_expression *val)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, RIR_EXPRESSION_RETURN);
    ret->ret.val = val;
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
        if (!rf_stringx_append(r->buff, RFS("return"))) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ADD:
        if (!rf_stringx_append(r->buff, RFS("add"))) {
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
        if (!rf_stringx_append(r->buff, RFS("NOT_IMPLEMENTED"))) {
            goto end;
        }
        break;
    }

    if (!rf_stringx_append(r->buff, RFS("\n"))) {
        goto end;
    }
    //success
    ret = true;
end:
    RFS_POP();
    return ret;
}
