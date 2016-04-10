#include <ir/rir_utils.h>

#include <rflib/utils/log.h>

#include <ir/rir.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>
#include <ir/rir_expression.h>
#include <ir/rir_object.h>

#include <ast/constants.h>

struct rir_value g_rir_const_1;
struct rir_value g_rir_const_m1;
struct rir_type g_rir_i32_type;
static bool utils_created = false;

static bool rir_static_constantval_init_fromint32(struct rir_value *v, int32_t n)
{
    struct ast_constant c;
    v->category = RIR_VALUE_CONSTANT;
    ast_constant_init_int(&c, n);
    return rir_value_static_constant_init(v, &c, &g_rir_i32_type);
}

bool rir_utils_create()
{
    // first create some global statuc types that will be useful elsewhere
    rir_type_elem_init(&g_rir_i32_type, ELEMENTARY_TYPE_INT_32, false);
    if (!rir_static_constantval_init_fromint32(&g_rir_const_1, 1)) {
        return false;
    }
    if (!rir_static_constantval_init_fromint32(&g_rir_const_m1, -1)) {
        return false;
    }
    utils_created = true;
    return true;
}

void rir_utils_destroy()
{
    if (utils_created) {
        rir_value_deinit(&g_rir_const_m1);
        rir_value_deinit(&g_rir_const_1);
    }
    utils_created = false;
}

struct rir_value *rirctx_getread_val(struct rir_value *v, struct rir_ctx *ctx)
{
    // if a pointer and not a string, also perform a read
    if (!v->type->is_pointer || rir_type_is_specific_elementary(v->type, ELEMENTARY_TYPE_STRING)) {
        return v;
    }
    struct rir_expression *read;
    read = rir_read_create(v, RIRPOS_AST, ctx);
    if (!read) {
        RF_ERROR("Failed to create a read RIR instruction");
        return NULL;
    }
    rir_common_block_add(&ctx->common, read);
    return &read->val;
}

struct rir_value *rirctx_getread_exprval(struct rir_expression *e, struct rir_ctx *ctx)
{
    struct rir_expression *ret = rirctx_getread_expr(e, ctx);
    return ret ? &ret->val : NULL;
}

struct rir_expression *rirctx_getread_expr(struct rir_expression *e, struct rir_ctx *ctx)
{
    struct rir_expression *ret = NULL;
    // gotta read the memory value from an alloca
    // unless it's a string, which is passed by pointer at least at the moment
    if (e->type == RIR_EXPRESSION_ALLOCA &&
        !rir_type_is_specific_elementary(e->alloca.type, ELEMENTARY_TYPE_STRING)
    ) {
        struct rir_expression *read;
        read = rir_read_create(&e->val, RIRPOS_AST, ctx);
        if (!read) {
            RF_ERROR("Failed to create a read RIR instruction");
            return NULL;
        }
        rir_common_block_add(&ctx->common, read);
        ret = read;
    } else {
        ret = e;
    }
    return ret;
}

struct rir_object *rirctx_getread_obj(struct rir_object *o, struct rir_ctx *ctx)
{
    struct rir_object *ret = NULL;
    RF_ASSERT(o->category == RIR_OBJ_EXPRESSION, "Expected an expression object");
    struct rir_expression *e = &o->expr;
    // gotta read the memory value from an alloca
    // unless it's a string, which is passed by pointer at least at the moment
    if (
        e->type == RIR_EXPRESSION_ALLOCA &&
        !rir_type_is_specific_elementary(e->alloca.type, ELEMENTARY_TYPE_STRING)
    ) {
        struct rir_object *read;
        read = rir_read_create_obj(&e->val, RIRPOS_AST, ctx);
        if (!read) {
            RF_ERROR("Failed to create a read RIR instruction");
            return NULL;
        }
        rir_common_block_add(&ctx->common, &read->expr);
        ret = read;
    } else {
        ret = o;
    }
    return ret;
}
