#include <ir/rir_expression.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_call.h>
#include <ir/rir_value.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_function.h>
#include <ir/rir_argument.h>
#include <ir/rir_constant.h>
#include <ir/rir_call.h>
#include <Utils/sanity.h>
#include <ast/ast.h>

void rir_expression_init_with_nilval(struct rir_expression *e,
                                     enum rir_expression_type type)
{
    RF_ASSERT(type == RIR_EXPRESSION_WRITE ||
              type == RIR_EXPRESSION_RETURN ||
              type == RIR_EXPRESSION_SETUNIONIDX,
              "Expression which should not have a nil value given to function");
    e->type = type;
    rir_value_nil_init(&e->val);
}

bool rir_expression_init(struct rir_object *obj,
                         enum rir_expression_type type,
                         struct rir_ctx *ctx)
{
    struct rir_expression *expr = &obj->expr;
    expr->type = type;
    switch (obj->expr.type) {
    case RIR_EXPRESSION_CONSTANT:
        RF_CRITICAL_FAIL("Rir constants should only be initialized by rir_constant_create");
        break;
    case RIR_EXPRESSION_WRITE:
    case RIR_EXPRESSION_RETURN:
    case RIR_EXPRESSION_SETUNIONIDX:
        rir_value_nil_init(&expr->val);
        break;
    default:
        if (!rir_value_variable_init(&expr->val, obj, NULL, ctx)) {
            return false;
        }
    }
    return true;
}


void rir_expression_deinit(struct rir_expression *expr)
{
    switch (expr->type) {
    case RIR_EXPRESSION_CALL:
        rir_call_deinit(&expr->call);
        break;
    case RIR_EXPRESSION_ALLOCA:
        rir_ltype_destroy(expr->alloca.type);
        break;
    default:
        break;
    }
    rir_value_deinit(&expr->val);
}

static inline void rir_alloca_init(struct rir_alloca *obj,
                                   struct rir_ltype *type,
                                   uint64_t num)
{
    obj->type = type;
    obj->num = num;
}

static struct rir_object *rir_read_create_obj(const struct rir_value *memory_to_read,
                                              struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        free(ret);
        return NULL;
    }
    ret->expr.read.memory = memory_to_read;
    if (!rir_expression_init(ret, RIR_EXPRESSION_READ, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_read_create(const struct rir_value *memory_to_read,
                                   struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_read_create_obj(memory_to_read, ctx);
    return obj ? &obj->expr : NULL;
}

struct rir_object *rir_alloca_create_obj(struct rir_ltype *type,
                                         uint64_t num,
                                         struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    rir_alloca_init(&ret->expr.alloca, type, num);
    if (!rir_expression_init(ret, RIR_EXPRESSION_ALLOCA, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static inline void rir_alloca_deinit(struct rir_expression *obj)
{
    return;// TODO
}

static struct rir_object *rir_setunionidx_create_obj(const struct rir_value *unimemory,
                                                     const struct rir_value *idx,
                                                     struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.setunionidx.unimemory = unimemory;
    ret->expr.setunionidx.idx = idx;
    if (!rir_expression_init(ret, RIR_EXPRESSION_SETUNIONIDX, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_setunionidx_create(const struct rir_value *unimemory,
                                              const struct rir_value *idx,
                                              struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_setunionidx_create_obj(unimemory, idx, ctx);
    return obj ? &obj->expr : NULL;
}

static struct rir_object *rir_getunionidx_create_obj(const struct rir_value *unimemory,
                                                     struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.getunionidx.unimemory = unimemory;
    if (!rir_expression_init(ret, RIR_EXPRESSION_GETUNIONIDX, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_getunionidx_create(const struct rir_value *unimemory,
                                              struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_getunionidx_create_obj(unimemory, ctx);
    return obj ? &obj->expr : NULL;
}

struct rir_object *rir_objmemberat_create_obj(const struct rir_value *objmemory,
                                              uint32_t idx,
                                              struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.objmemberat.objmemory = objmemory;
    ret->expr.objmemberat.idx = idx;
    if (!rir_expression_init(ret, RIR_EXPRESSION_OBJMEMBERAT, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_objmemberat_create(const struct rir_value *objmemory,
                                              uint32_t idx,
                                              struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_objmemberat_create_obj(objmemory, idx, ctx);
    return obj ? &obj->expr : NULL;
}

static struct rir_object *rir_unionmemberat_create_obj(const struct rir_value *unimemory,
                                                       uint32_t idx,
                                                       struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.unionmemberat.unimemory = unimemory;
    ret->expr.unionmemberat.idx = idx;
    if (!rir_expression_init(ret, RIR_EXPRESSION_UNIONMEMBERAT, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_unionmemberat_create(const struct rir_value *unimemory,
                                                uint32_t idx,
                                                struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_unionmemberat_create_obj(unimemory, idx, ctx);
    return obj ? &obj->expr : NULL;
}

void rir_return_init(struct rir_expression *ret,
                     const struct rir_expression *val)
{
    ret->ret.val = val;
    rir_expression_init_with_nilval(ret, RIR_EXPRESSION_RETURN);
}

static struct rir_object *rir_convert_create_obj(const struct rir_ltype *totype,
                                                 const struct rir_value *convval,
                                                 struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        return NULL;
    }
    ret->expr.convert.totype = totype;
    ret->expr.convert.convval = convval;
    if (!rir_expression_init(ret, RIR_EXPRESSION_CONVERT, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_expression *rir_convert_create(const struct rir_ltype *totype,
                                          const struct rir_value *convval,
                                          struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_convert_create_obj(totype, convval, ctx);
    return obj ? &obj->expr : NULL;
}


bool rir_expression_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();
    switch(e->type) {
    case RIR_EXPRESSION_CALL:
        if (!rir_call_tostring(ctx, e)) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_SETUNIONIDX:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT"setunionidx(" RF_STR_PF_FMT ", %" PRId64 ")\n",
                    RF_STR_PF_ARG(rir_value_string(e->setunionidx.unimemory)),
                    e->setunionidx.idx->constant.value.integer)
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_GETUNIONIDX:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT " = getunionidx(" RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_value_string(e->getunionidx.unimemory)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_OBJMEMBERAT:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = objmemberat(" RF_STR_PF_FMT ", %" PRId32 ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_value_string(e->objmemberat.objmemory)),
                    e->setunionidx.idx)
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_UNIONMEMBERAT:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = unionmemberat(" RF_STR_PF_FMT ", %" PRId32 ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_value_string(e->unionmemberat.unimemory)),
                    e->setunionidx.idx)
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_ALLOCA:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = alloca(" RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_ltype_string(e->alloca.type)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_READ:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = read(" RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_value_string(e->read.memory)))
            )) {
            goto end;
        }
        break;
    case RIR_EXPRESSION_CONVERT:
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = convert("RF_STR_PF_FMT", "RF_STR_PF_FMT")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(rir_ltype_string(e->convert.totype)),
                    RF_STR_PF_ARG(rir_value_string(e->convert.convval))
                ))) {
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
