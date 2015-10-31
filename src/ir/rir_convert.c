#include <ir/rir_convert.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir_process.h>
#include <ir/rir_constant.h>
#include <ir/rir_function.h>
#include <ir/rir.h>

#include <utils/common_strings.h>

#include <ast/function.h>

static struct rir_object *rir_convert_init(const struct rir_value *convval,
                                           const struct rir_type *totype,
                                           struct rir_ctx *ctx)
{
    struct rir_object *retobj = NULL;
    // at the moment only conversion to string requires special work
    if (!rir_type_is_specific_elementary(totype, ELEMENTARY_TYPE_STRING)) {
        if ((!(retobj = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir)))) {
            return NULL;
        }
        retobj->expr.convert.val = convval;
        retobj->expr.convert.type = totype;
        retobj->expr.type = RIR_EXPRESSION_PLACEHOLDER; // to signify we must continue
        return retobj;
    }

    // from here and down it's only about conversion to string
    if (convval->category == RIR_VALUE_CONSTANT) {
        // constant value to string conversion can be done easily here at compile time
        RFS_PUSH();
        const struct RFstring *temps = rir_constant_string(convval);
        if (!(retobj = rir_global_addorget_string(ctx, temps))) {
            RF_ERROR("Failed to add or get a global string literal to the RIR");
        }
        RFS_POP();
        return retobj;
    } else if (rir_type_is_specific_elementary(convval->type, ELEMENTARY_TYPE_BOOL)) {
        struct rir_object *obj;
        // boolean to string conversion requires some branching logic
        retobj = rir_alloca_create_obj(
            rir_type_elem_create(ELEMENTARY_TYPE_STRING, false),
            0,
            ctx
        );
        if (!retobj) {
            return NULL;
        }
        rirctx_block_add(ctx, &retobj->expr);
        struct rir_value *allocv = rir_object_value(retobj);
        // create the blocks
        struct rir_block *prev_block = ctx->current_block;
        struct rir_block *taken_block = rir_block_create(NULL, false, ctx);
        if (!taken_block) {
            return NULL;
        }
        struct rir_block *fallthrough_block = rir_block_create(NULL, false, ctx);
        if (!fallthrough_block) {
            return NULL;
        }
        struct rir_block *after_block = rir_block_create(NULL, false, ctx);
        if (!after_block) {
            return NULL;
        }

        // create the conditional branch to connect to if/else
        if (!rir_block_exit_init_condbranch(
                &prev_block->exit, convval, &taken_block->label, &fallthrough_block->label
            )) {
            return NULL;
        }

        // populate taken block
        ctx->current_block = taken_block;
        if (!(obj = rir_global_addorget_string(ctx, &g_str_true))) {
            RF_ERROR("Failed to add a global string literal to the RIR");
            return NULL;
        }
        struct rir_expression *e = rir_write_create(allocv, rir_object_value(obj), ctx);
        if (!e) {
            return NULL;
        }
        rirctx_block_add(ctx, e);
        if (!rir_block_exit_init_branch(&taken_block->exit, &after_block->label)) {
            return NULL;
        }
        rir_fndef_add_block(ctx->current_fn, taken_block);

        // populate fallthrough block
        ctx->current_block = fallthrough_block;
        if (!(obj = rir_global_addorget_string(ctx, &g_str_false))) {
            RF_ERROR("Failed to add a global string literal to the RIR");
            return NULL;
        }
        if (!(e = rir_write_create(allocv, rir_object_value(obj), ctx))) {
            return NULL;
        }
        rirctx_block_add(ctx, e);
        if (!rir_block_exit_init_branch(&fallthrough_block->exit, &after_block->label)) {
            return NULL;
        }
        rir_fndef_add_block(ctx->current_fn, fallthrough_block);

        // finally let's go to the after block and return the populated alloca which
        // should hold the value of the conversion
        rir_fndef_add_block(ctx->current_fn, after_block);
        ctx->current_block = after_block;
        return retobj;
    } else {
        RF_CRITICAL_FAIL("Unexpected conversion at RIR formation");
        return NULL;
    }
}

struct rir_object *rir_convert_create_obj(const struct rir_value *convval,
                                          const struct rir_type *totype,
                                          struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_convert_init(convval, totype, ctx);
    if (!ret || !(ret->category == RIR_OBJ_EXPRESSION && ret->expr.type == RIR_EXPRESSION_PLACEHOLDER)) {
        // if error, or conversion created additional rir instructions and not just a simple convert
        return ret;
    }
    if (!rir_object_expression_init(ret, RIR_EXPRESSION_CONVERT, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_object *rir_convert_create_obj_maybeadd(const struct rir_value *convval,
                                                   const struct rir_type *totype,
                                                   struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_convert_create_obj(convval, totype, ctx);
    if (!obj) {
        RF_ERROR("Failed to create convert rir instruction object");
        return NULL;
    }
    if (obj->category == RIR_OBJ_EXPRESSION && obj->expr.type == RIR_EXPRESSION_CONVERT) {
        rirctx_block_add(ctx, &obj->expr);
    }
    return obj;
}

const struct rir_value *rir_maybe_convert(const struct rir_value *val,
                                          const struct rir_type *checktype,
                                          struct rir_ctx *ctx)
{
    if (!rir_type_equal(val->type, checktype)) {
        struct rir_object *obj;
        if (!(obj = rir_convert_create_obj_maybeadd(val, rir_type_create_from_other(checktype, false), ctx))) {
            return NULL;
        }
        val = rir_object_value(obj);
    }
    return val;
}


bool rir_process_convertcall(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct ast_node *args = ast_fncall_args(n);
    RF_ASSERT(ast_node_get_type(args)->category != TYPE_CATEGORY_OPERATOR,
              "A conversion call should only have a single argument");
    // process that argument
    const struct rir_value *argexprval = rir_process_ast_node_getreadval(args, ctx);
    if (!argexprval) {
        RF_ERROR("Could not create rir expression from conversion call argument");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    // create the conversion
    struct rir_object *obj = rir_convert_create_obj_maybeadd(
        argexprval,
        rir_type_create_from_type(ast_node_get_type(n), ctx),
        ctx
    );
    if (!obj) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, obj);
}
