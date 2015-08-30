#include <ir/rir_value.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_expression.h>
#include <String/rf_str_core.h>
#include <String/rf_str_manipulationx.h>
#include <Utils/memory.h>

bool rir_value_label_init_string(struct rir_value *v, struct rir_block *b, const struct RFstring *s, struct rir_ctx *ctx)
{
    v->type = RIR_VALUE_LABEL;
    v->label_dst = b;
    if (!rf_string_copy_in(&v->id, s)) {
        return false;
    }
    return rir_strmap_addblock_from_id(ctx, &v->id, b);
}

bool rir_value_constant_init(struct rir_value *v, const struct ast_constant *c)
{
    bool ret;
    v->constant = *c;
    switch (v->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        ret = rf_string_initv(&v->id, "%"PRId64, v->constant.value.integer);
        break;
    case CONSTANT_NUMBER_FLOAT:
        ret = rf_string_initv(&v->id, "%f", v->constant.value.floating);
        break;
    case CONSTANT_BOOLEAN:
        ret = rf_string_initv(&v->id, "%s", v->constant.value.boolean ? "true" : "false");
        break;
    }
    return ret;
}

bool rir_value_init(struct rir_value *v, enum rir_valtype type, void *obj, struct rir_ctx *ctx)
{
    struct rir_expression *e;
    struct rir_block *b;
    bool ret = true;
    v->type = type;
    switch (v->type) {
    case RIR_VALUE_CONSTANT:
        e = obj;
        ret = rir_value_constant_init(v, &e->constant);
        break;
    case RIR_VALUE_VARIABLE:
        e = obj;
        v->expr = e;
        if (!rf_string_initv(&v->id, "$%d", ctx->expression_idx++)) {
            return false;
        }
        ret = rir_strmap_addexpr_from_id(ctx, &v->id, e);
        break;
    case RIR_VALUE_LABEL:
        b = obj;
        v->label_dst = b;
        if (!rf_string_initv(&v->id, "%%label_%d", ctx->label_idx++)) {
            return false;
        }
        ret = rir_strmap_addblock_from_id(ctx, &v->id, b);
        break;
    case RIR_VALUE_NIL:
        // nothing to init for nil value
        break;
    default:
        RF_ASSERT(false, "Should not get here");
        break;
    }
    return ret;
}

struct rir_value *rir_value_create(enum rir_valtype type, struct rir_expression *e, struct rir_ctx *ctx)
{
    struct rir_value *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_value_init(ret, type, e, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_value_deinit(struct rir_value *v)
{
    
}

void rir_value_destroy(struct rir_value *v)
{
    rir_value_deinit(v);
    free(v);
}

bool rir_value_tostring(struct rir *r, const struct rir_value *v)
{
    switch (v->type) {
    case RIR_VALUE_CONSTANT:
    case RIR_VALUE_VARIABLE:
    case RIR_VALUE_LABEL:
        if (!rf_stringx_append(r->buff, &v->id)) {
            return false;
        }
    case RIR_VALUE_NIL:
        break;
    }
    return true;
}

const struct RFstring *rir_value_string(const struct rir_value *v)
{
    static const struct RFstring empty = RF_STRING_STATIC_INIT("");
    switch (v->type) {
    case RIR_VALUE_CONSTANT:
    case RIR_VALUE_VARIABLE:
    case RIR_VALUE_LABEL:
        return &v->id;
    case RIR_VALUE_NIL:
        break;
    }
    return &empty;
}
  
