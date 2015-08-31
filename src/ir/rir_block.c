#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <ir/rir_value.h>
#include <ir/rir_function.h>
#include <ir/rir_call.h>
#include <ir/rir_constant.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_strmap.h>
#include <ast/ast.h>
#include <ast/ifexpr.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/operators.h>
#include <ast/returnstmt.h>
#include <types/type.h>

/* -- functions for rir_block_exit -- */
bool rir_block_exit_init_branch(struct rir_block_exit *exit,
                                struct rir_value *branch_dst)
{
    exit->type = RIR_BLOCK_EXIT_BRANCH;
    return rir_branch_init(&exit->branch, branch_dst);
}

bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                    struct rir_expression *cond,
                                    struct rir_value *taken,
                                    struct rir_value *fallthrough)
{
    exit->type = RIR_BLOCK_EXIT_CONDBRANCH;
    return rir_condbranch_init(&exit->condbranch, cond, taken, fallthrough);
}

static inline void rir_block_exit_deinit(struct rir_block_exit *exit)
{
    switch (exit->type) {
    case RIR_BLOCK_EXIT_BRANCH:
        rir_branch_deinit(&exit->branch);
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        rir_condbranch_deinit(&exit->condbranch);
        break;
    case RIR_BLOCK_EXIT_RETURN:
    case RIR_BLOCK_EXIT_INVALID:
        // TODO
        RF_ASSERT(false, "Not yet implemented");
        break;
    }
}

bool rir_block_exit_return_init(struct rir_block_exit *exit,
                                const struct rir_expression *val,
                                struct rir_ctx *ctx)
{
    exit->type = RIR_BLOCK_EXIT_RETURN;
    return rir_return_init(&exit->retstmt, val, ctx);
}

static bool rir_blockexit_tostring(struct rirtostr_ctx *ctx, const struct rir_block_exit *exit)
{
    bool ret = false;
    RFS_PUSH();
    switch (exit->type) {
    case RIR_BLOCK_EXIT_BRANCH:
        if (exit->branch.dst) {
            if (!rir_branch_tostring(ctx, &exit->branch)) {
                goto end;
            }
        } else {
            rf_stringx_append_cstr(ctx->rir->buff, "branch(NODESTINATION-FIXME)\n");
        }
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        if (!rir_condbranch_tostring(ctx, &exit->condbranch)) {
            goto end;
        }
        break;
    case RIR_BLOCK_EXIT_RETURN:
        if (exit->retstmt.ret.val) {
            if (!rf_stringx_append(
                    ctx->rir->buff,
                    RFS(RITOSTR_INDENT"return("RF_STR_PF_FMT")\n",
                        RF_STR_PF_ARG(rir_value_string(&exit->retstmt.ret.val->val)))
                )) {
                goto end;
            }
        } else {
            if (!rf_stringx_append_cstr(ctx->rir->buff, RITOSTR_INDENT"return()\n")) {
                goto end;
            }
        }
        break;
    case RIR_BLOCK_EXIT_INVALID:
        RF_ASSERT_OR_EXIT(false, "Should never happen");
        goto end;
    }

    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}


/* -- functions for rir_block -- */
static struct rir_block *rir_process_elif(const struct ast_node *n, struct rir_ctx *ctx);
static struct rir_block *rir_process_elif(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_block *old_block = ctx->current_block;
    if (!(old_block = rir_block_create(NULL, false, ctx))) {
        goto fail;
    }
    rir_fndecl_add_block(ctx->current_fn, old_block);

    if (!rir_process_ast_node(ast_ifexpr_condition_get(n), ctx)) {
        goto fail;
    }
    struct rir_expression *cond = ctx->returned_expr;
    struct rir_block *taken = rir_block_create(ast_ifexpr_taken_block_get(n), false, ctx);
    if (!taken) {
        goto fail;
    }

    struct rir_block *else_block = NULL;
    struct ast_node *fallthrough_branch = ast_ifexpr_fallthrough_branch_get(n);
    struct rir_block *new_if_block;
    if (fallthrough_branch) {
        if (fallthrough_branch->type == AST_IF_EXPRESSION) {
            if (!(new_if_block = rir_process_elif(fallthrough_branch, ctx))) {
                goto fail;
            }
        } else {
            // should be a block
            else_block = rir_block_create(fallthrough_branch, false, ctx);
        }
    }

    struct rir_block *new_block;
    if (fallthrough_branch) {
        new_block = fallthrough_branch->type == AST_IF_EXPRESSION
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
static bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_block *old_block = ctx->current_block;
    RF_ASSERT(!ctx->next_block, "next block should be empty here");
    if (!(ctx->next_block = rir_block_create(NULL, false, ctx))) {
        goto fail;
    }
    ctx->current_block = old_block;

    if (!rir_process_ast_node(ast_ifexpr_condition_get(n), ctx)) {
        goto fail;
    }
    struct rir_expression *cond = ctx->returned_expr;
    struct rir_block *taken = rir_block_create(ast_ifexpr_taken_block_get(n), false, ctx);
    if (!taken) {
        goto fail;
    }

    struct rir_block *else_block = NULL;
    struct ast_node *fallthrough_branch = ast_ifexpr_fallthrough_branch_get(n);
    struct rir_block *new_if_block;
    if (fallthrough_branch) {
        if (fallthrough_branch->type == AST_IF_EXPRESSION) {
            if (!(new_if_block = rir_process_elif(fallthrough_branch, ctx))) {
                goto fail;
            }
        } else {
            // should be a block
            else_block = rir_block_create(fallthrough_branch, false, ctx);
        }
    }

    struct rir_block *new_block = NULL;
    if (fallthrough_branch) {
        new_block = fallthrough_branch->type == AST_IF_EXPRESSION
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

    ctx->current_block = ctx->next_block;
    // since next_block was an empty block let's add it to the function now
    rir_fndecl_add_block(ctx->current_fn, ctx->next_block);
    ctx->next_block = NULL;
    RIRCTX_RETURN_EXPR(ctx, true, NULL);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

static bool rir_process_vardecl(const struct ast_node *n,
                                struct rir_ctx *ctx)
{
    struct ast_node *left = ast_types_left(ast_vardecl_desc_get(n));
    const struct RFstring *id_str = ast_identifier_str(left);
    RFS_PUSH();
    struct RFstring *type_str = type_str_or_die(ast_node_get_type_or_die(left, AST_TYPERETR_DEFAULT), TSTR_DEFAULT);
    struct rir_ltype *allocated_type = rir_type_byname(ctx->rir, type_str);
    RFS_POP();
    if (!allocated_type) {
        RF_ERROR("Could not find the type that an alloc() command should allocate in the RIR");
        return false;
    }
    struct rir_expression *alloca = rir_alloca_create(
        allocated_type,
        1,
        ctx
    );
    if (!alloca) {
        goto fail;
    }
    if (!strmap_add(&ctx->current_fn->id_map, id_str, alloca)) {
        goto fail;
    }
    rirctx_block_add(ctx, alloca);
    RIRCTX_RETURN_EXPR(ctx, true, alloca);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

static bool rir_process_return(const struct ast_node *n,
                               struct rir_ctx *ctx)
{
    const struct RFstring returnval_str = RF_STRING_STATIC_INIT("$returnval");
    if (!rir_process_ast_node(ast_returnstmt_expr_get(n), ctx)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *ret_val = ctx->returned_expr;
    // write the return value to the return slot
    struct rir_expression *ret_slot = strmap_get(&ctx->current_fn->id_map, &returnval_str);
    if (!ret_slot) {
        RF_ERROR("Could not find the returnvalue of a function in the string map");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *e = rir_binaryop_create_nonast(RIR_EXPRESSION_WRITE, &ret_slot->val, &ret_val->val, ctx);
    rirctx_block_add(ctx, e);
    // jump to the return
    if (!rir_block_exit_init_branch(&ctx->current_block->exit, ctx->current_fn->end_label)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }

    RIRCTX_RETURN_EXPR(ctx, true, NULL);
}

static bool rir_process_constant(const struct ast_node *n,
                                 struct rir_ctx *ctx)
{
    struct rir_expression *ret_expr = rir_constant_create(n, ctx);
    RIRCTX_RETURN_EXPR(ctx, true, ret_expr);
}

bool rir_process_identifier(const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    struct rir_expression *expr = strmap_get(&ctx->current_fn->id_map, ast_identifier_str(n));
    if (!expr) {
        RF_ERROR("An identifier was not found in the strmap during rir creation");
        return NULL;
    }
    RIRCTX_RETURN_EXPR(ctx, true, expr);
}

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx)
{
    switch (n->type) {
    case AST_IF_EXPRESSION:
        return rir_process_ifexpr(n, ctx);
    case AST_VARIABLE_DECLARATION:
        return rir_process_vardecl(n, ctx);
    case AST_BINARY_OPERATOR:
        return rir_process_binaryop(&n->binaryop, ctx);
    case AST_RETURN_STATEMENT:
        return rir_process_return(n, ctx);
    case AST_CONSTANT:
        return rir_process_constant(n, ctx);
    case AST_IDENTIFIER:
        return rir_process_identifier(n, ctx);
    case AST_FUNCTION_CALL:
        return rir_process_fncall(n, ctx);
    case AST_MATCH_EXPRESSION:
        // TODO: implement properly
    case AST_CONDITIONAL_BRANCH:
    case AST_MATCH_CASE:
        // Do nothing in these cases
        return true;
    default:
        RF_ASSERT(false, "Not yet implemented expression for RIR");
    }
    return false;
}

struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx)
{
    const struct RFstring fend_label = RF_STRING_STATIC_INIT("%function_end");
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    RF_STRUCT_ZERO(ret);
    ctx->current_block = ret;
    rf_ilist_head_init(&ret->expressions);
    if (!rir_value_label_init_string(&ret->label, ret, &fend_label, ctx)) {
        free (ret);
        ret = NULL;
    }

    // current block's exit should be the return
    const struct RFstring returnval_str = RF_STRING_STATIC_INIT("$returnval");
    struct rir_expression *ret_slot = NULL;
    if (has_return) {
        ret_slot = strmap_get(&ctx->current_fn->id_map, &returnval_str);
        if (!ret_slot) {
            RF_ERROR("Could not find the returnvalue of a function in the string map");
            RIRCTX_RETURN_EXPR(ctx, false, NULL);
        }
    }
    if (!rir_block_exit_return_init(&ret->exit, ret_slot, ctx)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    return ret;
}

static bool rir_block_init(struct rir_block *b,
                           const struct ast_node *n,
                           bool function_beginning,
                           struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    ctx->current_block = b;
    if (!function_beginning) {
        if (!rir_value_init(&b->label, RIR_VALUE_LABEL, b, ctx)) {
            return false;
        }
    } else {
        if (!rir_value_init(&b->label, RIR_VALUE_NIL, NULL, ctx)) {
            return false;
        }
    }

    struct ast_node *child;
    if (n) {
        // add basic block to the current function
        rir_fndecl_add_block(ctx->current_fn, b);
        // TODO: for now let's accept match expressions here too, but the body should be created
        // in quite a different way for match expressions so it would probably become its own function
        RF_ASSERT(n->type == AST_BLOCK || n->type == AST_MATCH_EXPRESSION,
                  "rir_block_init() called with invalid ast node type");
        ctx->current_ast_block = n;
        // for each expression of the block create a rir expression and add it to the block
        rf_ilist_for_each(&n->children, child, lh) {
            if (!rir_process_ast_node(child, ctx)) {
                return false;
            }
        }
    }

    return true;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   bool function_beginning,
                                   struct rir_ctx *ctx)
{
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_block_init(ret, n, function_beginning, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_block_deinit(struct rir_block* b)
{
    struct rir_expression *expr;
    struct rir_expression *tmp;
    rf_ilist_for_each_safe(&b->expressions, expr, tmp, ln) {
        rir_expression_destroy(expr);
    }
    rir_block_exit_deinit(&b->exit);
}

void rir_block_destroy(struct rir_block* b)
{
    rir_block_deinit(b);
    free(b);
}

bool rir_block_tostring(struct rirtostr_ctx *ctx, const struct rir_block *b)
{
    struct rir_expression *expr;
    rirtostr_ctx_visit_block(ctx, b);
    if (b->label.type == RIR_VALUE_LABEL) {
        if (!rir_value_tostring(ctx->rir, &b->label)) {
            return false;
        }
        if (!rf_stringx_append_cstr(ctx->rir->buff, "\n")) {
            return false;
        }
    }
    rf_ilist_for_each(&b->expressions, expr, ln) {
        if (!rir_expression_tostring(ctx, expr)) {
                return false;
        }
    }

    if (!rir_blockexit_tostring(ctx, &b->exit)) {
        return false;
    }

    return true;
}

i_INLINE_INS bool rir_block_exit_initialized(const struct rir_block *b);
