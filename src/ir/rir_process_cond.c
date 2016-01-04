#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_function.h>
#include <ir/rir_value.h>
#include <ast/ifexpr.h>

static struct rir_block *rir_process_elif(const struct ast_node *ast_cond,
                                          const struct ast_node *ast_taken,
                                          const struct ast_node *ast_fallthrough,
                                          struct rir_ctx *ctx);

/* -- functions for rir_block -- */
static struct rir_block *rir_process_conditional(
    struct rir_value *cond,
    const struct ast_node *ast_taken_block,
    const struct ast_node *ast_fallthrough_branch,
    struct rir_block *old_block,
    struct rir_ctx *ctx
)
{
    struct rir_block *taken = rir_block_create_from_ast(ast_taken_block, false, ctx);
    if (!taken) {
        goto fail;
    }

    struct rir_block *else_block = NULL;
    struct rir_block *new_if_block;
    if (ast_fallthrough_branch) {
        if (ast_fallthrough_branch->type == AST_IF_EXPRESSION) {
            if (!(new_if_block = rir_process_elif(
                      ast_ifexpr_condition_get(ast_fallthrough_branch),
                      ast_ifexpr_taken_block_get(ast_fallthrough_branch),
                      ast_ifexpr_fallthrough_branch_get(ast_fallthrough_branch),
                      ctx))) {
                goto fail;
            }
        } else {
            // should be a block
            else_block = rir_block_create_from_ast(ast_fallthrough_branch, false, ctx);
        }
    }

    struct rir_block *new_block = NULL;
    if (ast_fallthrough_branch) {
        new_block = ast_fallthrough_branch->type == AST_IF_EXPRESSION
            ? new_if_block
            : else_block;
    } else {
        new_block = ctx->next_block;
    }

    //Connect the old block with the new
    if (!rir_block_exit_init_condbranch(&old_block->exit, cond, &taken->label, &new_block->label)) {
        goto fail;
    }
    // if the taken block's exit is not initialized connect it to the after_if block
    if (!rir_block_exit_initialized(taken)) {
        if (!rir_block_exit_init_branch(&taken->exit, &ctx->next_block->label)) {
            goto fail;
        }
    }
    // if there was an else block connect it with the after_if block
    if (else_block) {
        if (!rir_block_exit_initialized(else_block)) {
            if (!rir_block_exit_init_branch(&else_block->exit, &ctx->next_block->label)) {
                goto fail;
            }
        }
    }

    return old_block;
fail:
    return NULL;
}

static struct rir_block *rir_process_conditional_ast(const struct ast_node *ast_cond,
                                                     const struct ast_node *ast_taken,
                                                     const struct ast_node *ast_fallthrough,
                                                     struct rir_block *old_block,
                                                     struct rir_ctx *ctx)
{
    const struct rir_value *cond = rir_process_ast_node_getreadval(ast_cond, ctx);
    if (!cond) {
        RF_ERROR("A condition value must have been created");
        return NULL;
    }
    return rir_process_conditional((struct rir_value*)cond, ast_taken, ast_fallthrough, old_block, ctx);
}

static struct rir_block *rir_process_elif(
    const struct ast_node *ast_cond,
    const struct ast_node *ast_taken,
    const struct ast_node *ast_fallthrough,
    struct rir_ctx *ctx
)
{
    struct rir_block *old_block;
    if (!(old_block = rir_block_create(NULL, false, ctx))) {
        return NULL;
    }
    rir_fndef_add_block(rir_ctx_curr_fn(ctx), old_block);
    return rir_process_conditional_ast(ast_cond, ast_taken, ast_fallthrough, old_block, ctx);
}

bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_block *old_block = rir_ctx_curr_block(ctx);
    if (!(ctx->next_block = rir_block_create(NULL, false, ctx))) {
        goto fail;
    }
    ctx->common.current_block = old_block;

    if (!rir_process_conditional_ast(ast_ifexpr_condition_get(n),
                                     ast_ifexpr_taken_block_get(n),
                                     ast_ifexpr_fallthrough_branch_get(n),
                                     old_block,
                                     ctx)) {
        goto fail;
    }

    ctx->common.current_block = ctx->next_block;
    // since next_block was an empty block let's add it to the function now
    rir_fndef_add_block(rir_ctx_curr_fn(ctx), ctx->next_block);
    RIRCTX_RETURN_EXPR(ctx, true, NULL);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}
