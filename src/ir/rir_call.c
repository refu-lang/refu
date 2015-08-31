#include <ir/rir_call.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_constant.h>
#include <ast/function.h>
#include <types/type.h>

struct args_to_val_ctx {
    struct rir_ctx *rir_ctx;
    unsigned index;
};

void args_to_val_ctx_init(struct args_to_val_ctx *ctx, struct rir_ctx *rir_ctx)
{
    ctx->rir_ctx = rir_ctx;
    ctx->index = 0;
}
static bool ctor_args_to_value_cb(const struct ast_node *n, struct args_to_val_ctx *ctx)
{
    // for each argument, process the ast node and create the rir arg expression
    if (!rir_process_ast_node(n, ctx->rir_ctx)) {
        return false;
    }
    if (!ctx->rir_ctx->returned_expr) {
        RF_ERROR("Could not create rir expression from constructor argument");
        return false;
    }
    struct rir_expression *argexpr = ctx->rir_ctx->returned_expr;
    if (!ctx->rir_ctx->last_assign_lhs) {
        RF_ERROR("RIR constructor call should have a valid left hand side in the assignment");
        return false;
    }
    // create a rir expression to read the object value at the assignee's index position
    struct rir_value *ririndexval = rir_constantval_fromint(ctx->index);
    struct rir_expression *readobj = rir_binaryop_create_nonast(
        RIR_EXPRESSION_OBJMEMBERAT,
        &ctx->rir_ctx->last_assign_lhs->val,
        ririndexval,
        ctx->rir_ctx
    );
    rirctx_block_add(ctx->rir_ctx, readobj);
    // write the arge expression to the position
    struct rir_expression *e = rir_binaryop_create_nonast(
        RIR_EXPRESSION_WRITE,
        &readobj->val,
        &argexpr->val,
        ctx->rir_ctx
    );
    rirctx_block_add(ctx->rir_ctx, e);
    
    ++ctx->index;
    return true;
}

bool rir_process_fncall(const struct ast_node *n, struct rir_ctx *ctx)
{
    const struct RFstring *fn_name = ast_fncall_name(n);
    const struct type *fn_type;
    // if we are in a block start check from there. If not simply search in the module
    const struct symbol_table *st = ctx->current_block
        ? &ctx->current_ast_block->block.st
        : ctx->current_module_st;
    fn_type = type_lookup_identifier_string(fn_name, st);
    if (!fn_type) {
        RF_ERROR("No function call of a given name could be found");
        return false;
    }

    struct args_to_val_ctx argsctx;
    args_to_val_ctx_init(&argsctx, ctx);
    if (fn_type->category == TYPE_CATEGORY_DEFINED) { // a constructor
        if (type_is_sumtype(fn_type)) {
            RF_ASSERT(false, "TODO");
        } else {
            ast_fncall_for_each_arg(n, (fncall_args_cb)ctor_args_to_value_cb, &argsctx);
        }
    }
    return true;
}
