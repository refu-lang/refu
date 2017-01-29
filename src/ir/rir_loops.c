#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_array.h>
#include <ir/rir_constant.h>
#include <ir/rir_binaryop.h>

#include <ast/forexpr.h>
#include <ast/iterable.h>

bool rir_process_forexpr(const struct ast_node *n, struct rir_ctx *ctx)
{
    // read the size of the iterable
    // TODO: Some work needed here when range is implemented
    if (!rir_process_identifier(
            ast_iterable_identifier_get(ast_forexpr_iterable_get(n)),
            ctx
        )) {
        goto fail;
    }
    struct rir_value *arr = rir_ctx_lastval_get(ctx);
    struct rir_object *sizeobj = rir_fixedarrsize_create(arr, ctx);
    if (!sizeobj) {
        RF_ERROR("Could not create fixedarrsize rir expression");
        goto fail;
    }
    rir_common_block_add(&ctx->common, &sizeobj->expr);

    // allocate and initialize the index to 0
    struct rir_object *indexobj = rir_alloca_create_obj(
        rir_type_elem_get_or_create(rir_ctx_rir(ctx), ELEMENTARY_TYPE_UINT_64, false),
        NULL,
        RIRPOS_AST,
        ctx
    );
    if (!indexobj) {
        goto fail;
    }
    rir_common_block_add(&ctx->common, &indexobj->expr);
    struct rir_value *zeroval = rir_constantval_create_fromint64(0, rir_ctx_rir(ctx));
    struct rir_expression *writezero = rir_write_create(
        rir_object_value(indexobj),
        zeroval,
        RIRPOS_AST,
        ctx
    );
    if (!writezero) {
        goto fail;
    }
    rir_common_block_add(&ctx->common, writezero);

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
    struct rir_expression *cmp = rir_binaryop_create_nonast(
        RIR_EXPRESSION_CMP_EQ,
        &curridx->val,
        rir_object_value(sizeobj),
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
        arr
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
