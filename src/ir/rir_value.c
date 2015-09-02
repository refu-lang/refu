#include <ir/rir_value.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <String/rf_str_core.h>
#include <String/rf_str_manipulationx.h>
#include <Utils/memory.h>

bool rir_value_label_init_string(struct rir_value *v, struct rir_block *b, const struct RFstring *s, struct rir_ctx *ctx)
{
    v->category = RIR_VALUE_LABEL;
    v->label_dst = b;
    if (!rf_string_copy_in(&v->id, s)) {
        return false;
    }
    return rir_strmap_addblock_from_id(ctx, &v->id, b);
}

bool rir_value_constant_init(struct rir_value *v, const struct ast_constant *c)
{
    bool ret;
    v->category = RIR_VALUE_CONSTANT;
    v->constant = *c;
    switch (v->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        v->type = rir_ltype_elem_create(ELEMENTARY_TYPE_INT_64, false);
        ret = rf_string_initv(&v->id, "%"PRId64, v->constant.value.integer);
        break;
    case CONSTANT_NUMBER_FLOAT:
        v->type = rir_ltype_elem_create(ELEMENTARY_TYPE_FLOAT_64, false);
        ret = rf_string_initv(&v->id, "%f", v->constant.value.floating);
        break;
    case CONSTANT_BOOLEAN:
        v->type = rir_ltype_elem_create(ELEMENTARY_TYPE_BOOL, false);
        ret = rf_string_initv(&v->id, "%s", v->constant.value.boolean ? "true" : "false");
        break;
    }
    return ret;
}

bool rir_value_variable_init(struct rir_value *v, struct rir_expression *e, struct rir_ctx *ctx)
{
    bool ret = false;
    v->category = RIR_VALUE_VARIABLE;
    v->expr = e;
    if (!rf_string_initv(&v->id, "$%d", ctx->expression_idx++)) {
        return false;
    }
    switch (v->expr->type) {
    case RIR_EXPRESSION_ALLOCA:
        v->type = rir_ltype_create_from_other(v->expr->alloca.type, true);
        break;
    case RIR_EXPRESSION_CMP:
        v->type = rir_ltype_elem_create(ELEMENTARY_TYPE_BOOL, false);
        break;
    case RIR_EXPRESSION_READ:
        if (!v->expr->read.memory->type->is_pointer) {
            RF_ERROR("Tried to rir read from a location not in memory");
            goto end;
        }
        v->type = rir_ltype_create_from_other(v->expr->alloca.type, false);
        break;
    case RIR_EXPRESSION_ADD:
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
        RF_ASSERT(rir_ltype_is_elementary(v->expr->binaryop.a->type), "Expected elementary type to be used in either part of rir binary op");
        v->type = rir_ltype_elem_create(v->expr->binaryop.a->type->etype, false);
        break;
    default:
        RF_ASSERT(false, "TODO: Unimplemented rir expression to value conversion");
        break;
    }
    // finally add it to the rir strmap
    ret = rir_strmap_addexpr_from_id(ctx, &v->id, e);
end:
    return ret;
}

bool rir_value_label_init(struct rir_value *v, struct rir_block *b, struct rir_ctx *ctx)
{
    v->category = RIR_VALUE_LABEL;
    v->label_dst = b;
    if (!rf_string_initv(&v->id, "%%label_%d", ctx->label_idx++)) {
        return false;
    }
    return rir_strmap_addblock_from_id(ctx, &v->id, b);
}

void rir_value_nil_init(struct rir_value *v)
{
    RF_STRUCT_ZERO(v);
    v->category = RIR_VALUE_NIL;
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
    switch (v->category) {
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
    switch (v->category) {
    case RIR_VALUE_CONSTANT:
    case RIR_VALUE_VARIABLE:
    case RIR_VALUE_LABEL:
        return &v->id;
    case RIR_VALUE_NIL:
        break;
    }
    return &empty;
}
  
