#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_array.h>
#include <ir/rir_constant.h>
#include <ir/rir_binaryop.h>

#include <ast/forexpr.h>
#include <ast/iterable.h>

/**
 * Convenience macro to read into the RIR either a constant range
 *  attribute or a variable one
 *
 * @param attribute_         One of start, step, end
 * @param iterable_          The ast_iterable node to read from
 * @param attribute_int_     An int64_t to read into
 * @param attribute_val_     A rir_value to read into
 */
#define iterable_range_attribute_to_rir(                                \
    attribute_,                                                         \
    iterable_,                                                          \
    attribute_int_,                                                     \
    attribute_val_                                                      \
)                                                                       \
    do {                                                                \
        if (ast_iterable_range_##attribute_##_get(iterable_, &attribute_int_)) { \
            /* attribute is constant */                                 \
            attribute_val_ = rir_constantval_create_fromint64(attribute_int_, rir_ctx_rir(ctx)); \
        } else {                                                        \
            /* read the attribute variable */                           \
            if (!rir_process_identifier(                                \
                    iterable->iterable.range.attribute_##_node,         \
                    ctx                                                 \
                )) {                                                    \
                goto fail;                                              \
            }                                                           \
            stepvalue = rir_ctx_lastval_get(ctx);                       \
        }                                                               \
    } while (0)

bool rir_process_forexpr(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct ast_node *iterable = ast_forexpr_iterable_get(n);
    struct rir_value *end_index_value;
    struct rir_value *iterablevalue = NULL;
    struct rir_object *indexobj;
    struct rir_value *stepvalue;
    struct rir_value *start_index_value;
    int64_t start_index = 0;
    int64_t step = 1;
    int64_t end;

    switch (ast_iterable_type_get(iterable)) {
    case ITERABLE_RANGE:
        RF_ASSERT(
            ast_iterable_range_start_get(iterable, &start_index),
            "Variable range not implemented yet"
        );

        iterable_range_attribute_to_rir(
            start,
            iterable,
            start_index,
            start_index_value
        );
        iterable_range_attribute_to_rir(step, iterable, step, stepvalue);
        iterable_range_attribute_to_rir(end, iterable, end, end_index_value);

        break;
    case ITERABLE_COLLECTION:
    {
        // read the size of the iterable
        if (!rir_process_identifier(
                ast_iterable_identifier_get(iterable),
                ctx
            )) {
            goto fail;
        }
        iterablevalue = rir_ctx_lastval_get(ctx);
        struct rir_object *sizeobj = rir_fixedarrsize_create(iterablevalue, ctx);
        if (!sizeobj) {
            RF_ERROR("Could not create fixedarrsize rir expression");
            goto fail;
        }
        rir_common_block_add(&ctx->common, &sizeobj->expr);
        // end index is the size of the array
        end_index_value = rir_object_value(sizeobj);

        // starting index is zero
        start_index_value = rir_constantval_create_fromint64(0, rir_ctx_rir(ctx));
        // step is constant
        stepvalue = rir_constantval_create_fromint64(step, rir_ctx_rir(ctx));
        break;
    }
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Should never get here");
        break;
    }

    // allocate, initialize and add the loop index to the block
    if (!(indexobj = rirctx_alloc_write_add(
            rir_type_elem_get_or_create(rir_ctx_rir(ctx), ELEMENTARY_TYPE_UINT_64, false),
            start_index_value,
            ctx))) {
        goto fail;
    }
    // allocate, initialize and add the step to the block
    if (!rirctx_alloc_write_add(
            rir_type_elem_get_or_create(rir_ctx_rir(ctx), ELEMENTARY_TYPE_UINT_64, false),
            stepvalue,
            ctx)) {
        goto fail;
    }

    // create the comparison block
    struct rir_block *old_block = rir_ctx_curr_block(ctx);
    struct rir_block *cmp_block = rir_block_create_from_ast(NULL, BLOCK_POSITION_NORMAL, ctx);
    if (!cmp_block) {
        goto fail;
    }
    ctx->common.current_block = cmp_block;
    rir_fndef_add_block(rir_ctx_curr_fn(ctx), cmp_block);
    // connect the old block with the comparison block
    if (!rir_block_exit_init_branch(&old_block->exit, &cmp_block->label)) {
        goto fail;
    }

    // read the current index value
    struct rir_expression *curridx = rir_read_create(rir_object_value(indexobj), RIRPOS_AST, ctx);
    if (!curridx) {
        goto fail;
    }
    rir_common_block_add(&ctx->common, curridx);
    // create a comparison of current index to iterable's size
    enum rir_expression_type comparison_type;
    if (ast_iterable_type_get(iterable) == ITERABLE_COLLECTION) {
        // when iterating array the index is simply increasing
        // by 1 until it reaches end index
        comparison_type = RIR_EXPRESSION_CMP_EQ;
    } else {
        if (start_index <= end) {
            comparison_type = RIR_EXPRESSION_CMP_GE;
        } else {
            comparison_type = RIR_EXPRESSION_CMP_LE;
        }
    }
    struct rir_expression *cmp = rir_binaryop_create_nonast(
        comparison_type,
        &curridx->val,
        end_index_value,
        RIRPOS_AST,
        ctx
    );
    if (!cmp) {
        goto fail;
    }
    rir_common_block_add(&ctx->common, cmp);

    // create the next/after block
    if (!(ctx->next_block = rir_block_create_from_ast(NULL, BLOCK_POSITION_NORMAL, ctx))) {
        goto fail;
    }
    ctx->common.current_block = cmp_block;


    // forward the loop related variables and create the loop body block
    rir_ctx_set_loopvars(
        ctx,
        ast_identifier_str(ast_forexpr_loopvar_get(n)),
        indexobj,
        iterablevalue,
        stepvalue
    );
    struct rir_block *loop_body = rir_block_create_from_ast(
        ast_forexpr_body_get(n),
        BLOCK_POSITION_LOOP,
        ctx
    );
    if (!loop_body) {
        goto fail;
    }

    // connect the loop body with the comparison block
    if (!rir_block_exit_init_branch(&loop_body->exit, &cmp_block->label)) {
        goto fail;
    }
    // conditionally connect the comparison block to the next label
    if (!rir_block_exit_init_condbranch(
            &cmp_block->exit,
            &cmp->val,
            &ctx->next_block->label,
            &loop_body->label
            )) {
        goto fail;
    }

    // since next_block was an empty block let's add it to the function now
    rir_fndef_add_block(rir_ctx_curr_fn(ctx), ctx->next_block);
    // also make it the current block
    ctx->common.current_block = ctx->next_block;
    RIRCTX_RETURN_EXPR(ctx, true, NULL);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}
