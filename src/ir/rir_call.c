#include <ir/rir_call.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_constant.h>
#include <ir/rir_argument.h>
#include <ir/rir_object.h>
#include <ast/function.h>
#include <types/type.h>

struct args_to_val_ctx {
    struct rir_ctx *rir_ctx;
    //! left hand of assignment
    struct rir_value *lhs;
    //! index of the argument we are iterating
    unsigned index;
};

static void args_to_val_ctx_init(struct args_to_val_ctx *ctx, struct rir_value *lhs, struct rir_ctx *rir_ctx)
{
    ctx->rir_ctx = rir_ctx;
    ctx->lhs = lhs;
    ctx->index = 0;
}
static bool ctor_args_to_value_cb(const struct ast_node *n, struct args_to_val_ctx *ctx)
{
    // for each argument, process the ast node and create the rir arg expression
    if (!rir_process_ast_node(n, ctx->rir_ctx)) {
        return false;
    }
    struct rir_value *argexprval = rir_ctx_lastval_get(ctx->rir_ctx);
    if (!argexprval) {
        RF_ERROR("Could not create rir expression from constructor argument");
        return false;
    }
    struct rir_value *lassignval = rir_ctx_lastassignval_get(ctx->rir_ctx);
    if (!lassignval) {
        RF_ERROR("RIR constructor call should have a valid left hand side in the assignment");
        return false;
    }
    // create a rir expression to read the object value at the assignee's index position
    struct rir_expression *readobj = rir_objmemberat_create(
        lassignval,
        ctx->index,
        ctx->rir_ctx
    );
    if (!readobj) {
        RF_ERROR("Failed to create rir expression to read an object's value");
        return false;
    }
    rirctx_block_add(ctx->rir_ctx, readobj);
    // write the arg expression to the position
    struct rir_expression *e = rir_binaryop_create_nonast(
        RIR_EXPRESSION_WRITE,
        &readobj->val,
        argexprval,
        ctx->rir_ctx
    );
    if (!e) {
        RF_ERROR("Failed to create expression to write to an object's member");
        return false;
    }
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

    if (fn_type->category == TYPE_CATEGORY_DEFINED) { // a constructor
        struct rir_value *lhs = rir_ctx_lastassignval_get(ctx);
        if (!lhs) {
            RF_ERROR("RIR constructor call should have a valid left hand side in the assignment");
            return false;
        }

        struct args_to_val_ctx argsctx;
        if (type_is_sumtype(fn_type)) {

            RF_ASSERT(rir_ltype_is_composite(lhs->type), "Constructor should assign to a composite type");
            int union_idx = rir_ltype_union_matched_type_from_fncall(lhs->type, n, ctx);
            if (union_idx == -1) {
                RF_ERROR("RIR sum constructor not matching any part of the original type");
                return false;
            }
            // create code to set the  union's index with the matching type
            struct rir_expression *e = rir_setunionidx_create(lhs, union_idx, ctx);
            if (!e) {
                return false;
            }
            rirctx_block_add(ctx, e);
            // create code to load the appropriate union subtype for reading
            e = rir_unionmemberat_create(lhs, union_idx, ctx);
            if (!e) {
                return false;
            }
            lhs = &e->val;
        }

        // now for whichever object (normal type, or union type sutype) is loaded as left hand side
        // assign from constructor's arguments
        args_to_val_ctx_init(&argsctx, lhs, ctx);
        ast_fncall_for_each_arg(n, (fncall_args_cb)ctor_args_to_value_cb, &argsctx);
    } else { // normal function call
        RF_ASSERT(false, "TODO");
    }
    return true;
}
