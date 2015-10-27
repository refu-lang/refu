#include <ir/rir_block.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir_value.h>
#include <ir/rir_function.h>
#include <ir/rir_process.h>
#include <ast/block.h>
#include <ast/matchexpr.h>
#include <utils/common_strings.h>

/* -- functions for rir_block_exit -- */
bool rir_block_exit_init_branch(struct rir_block_exit *exit,
                                struct rir_value *branch_dst)
{
    exit->type = RIR_BLOCK_EXIT_BRANCH;
    return rir_branch_init(&exit->branch, branch_dst);
}

bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                    const struct rir_value *cond,
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
    case RIR_BLOCK_EXIT_INVALID:
        RF_ASSERT(false, "Should never happen");
    case RIR_BLOCK_EXIT_RETURN:
        // the return stmt should be a global rir object so is freed from global rir objects list
        break;
    }
}

void rir_block_exit_return_init(struct rir_block_exit *exit,
                                const struct rir_expression *val)
{
    exit->type = RIR_BLOCK_EXIT_RETURN;
    rir_return_init(&exit->retstmt, val);
}

static bool rir_blockexit_tostring(struct rirtostr_ctx *ctx, const struct rir_block_exit *exitb)
{
    bool ret = false;
    RFS_PUSH();
    switch (exitb->type) {
    case RIR_BLOCK_EXIT_BRANCH:
        if (exitb->branch.dst) {
            if (!rir_branch_tostring(ctx, &exitb->branch)) {
                goto end;
            }
        } else {
            rf_stringx_append_cstr(ctx->rir->buff, "branch(NODESTINATION-FIXME)\n");
        }
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        if (!rir_condbranch_tostring(ctx, &exitb->condbranch)) {
            goto end;
        }
        break;
    case RIR_BLOCK_EXIT_RETURN:
        if (exitb->retstmt.ret.val) {
            if (!rf_stringx_append(
                    ctx->rir->buff,
                    RFS(RIRTOSTR_INDENT"return("RF_STR_PF_FMT")\n",
                        RF_STR_PF_ARG(rir_value_string(&exitb->retstmt.ret.val->val)))
                )) {
                goto end;
            }
        } else {
            if (!rf_stringx_append_cstr(ctx->rir->buff, RIRTOSTR_INDENT"return()\n")) {
                goto end;
            }
        }
        break;
    case RIR_BLOCK_EXIT_INVALID:
        RF_CRITICAL_FAIL("Should never happen");
        goto end;
    }

    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}

static struct rir_object *rir_block_functionend_create_obj(bool has_return, struct rir_ctx *ctx)
{
    const struct RFstring fend_label = RF_STRING_STATIC_INIT("function_end");
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, ctx->rir);
    if (!ret) {
        free(ret);
        ret = NULL;
    }
    struct rir_block *b = &ret->block;
    RF_STRUCT_ZERO(b);
    ctx->current_block = b;
    rf_ilist_head_init(&b->expressions);
    if (!rir_value_label_init_string(&ret->block.label, ret, &fend_label, ctx)) {
        free (ret);
        ret = NULL;
    }

    // current block's exit should be the return
    struct rir_expression *read_return = NULL;
    if (has_return) {
        read_return = rir_read_create(&ctx->current_fn->retslot_expr->val, ctx);
        if (!read_return) {
            RF_ERROR("Could not create a read from a function's return slot");
        }
        rirctx_block_add(ctx, read_return);
    }
    rir_block_exit_return_init(&ret->block.exit, read_return);
    return ret;
}

struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_functionend_create_obj(has_return, ctx);
    return obj ? &obj->block : NULL;
}

static struct rir_object *rir_block_matchcase_create_obj(const struct ast_node *mcase,
                                                         struct rir_object *matched_rir_obj,
                                                         struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, ctx->rir);
    if (!ret) {
        free(ret);
        return NULL;
    }
    struct rir_block *b = &ret->block;
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    ctx->current_block = b;
    if (!rir_value_label_init(&b->label, ret, false, ctx)) {
        goto fail;
    }
    AST_NODE_ASSERT_TYPE(mcase, AST_MATCH_CASE);
    // add basic block to the current function
    rir_fndef_add_block(ctx->current_fn, b);
    // populate the match case allocas
    if (!rir_match_st_populate_allocas(mcase, matched_rir_obj, ctx)) {
        RF_ERROR("Failed to populate a match case's alloca in the RIR");
        goto fail;
    }
    // finally process the contents of the match expression
    if (!rir_process_ast_node(ast_matchcase_expression(mcase), ctx)) {
        RF_ERROR("Failed to process a match case's expression in the RIR");
        goto fail;
    }
    return ret;

fail:
    free(ret);
    return NULL;
}

struct rir_block *rir_block_matchcase_create(const struct ast_node *mcase,
                                             struct rir_object *matched_rir_obj,
                                             struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_matchcase_create_obj(mcase, matched_rir_obj, ctx);
    return obj ? &obj->block : NULL;
}

/**
 * Initialize a rir block from an ast node
 *
 * @param obj                    The rir object of the block to get initialized
 * @param n                      An ast node expression. Can  be:
 *                               - ast_block:
 *                               in which case the ast_block creates an
 *                               equivalent rir block, or
 *                               - ast_expression
 *                               case a rir block with that expression is created.
 *                               - NULL
 *                               in which case an empty rir block is created
 * @param function_beginning     True if it's the first block of a function
 * @param ctx                    The rir context
 */
static bool rir_block_init(struct rir_object *obj,
                           const struct ast_node *n,
                           bool function_beginning,
                           struct rir_ctx *ctx)
{
    struct rir_block *b = &obj->block;
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    ctx->current_block = b;
    if (!rir_value_label_init(&b->label, obj, function_beginning, ctx)) {
            return false;
    }

    struct ast_node *child;
    if (n) {
        // add basic block to the current function
        rir_fndef_add_block(ctx->current_fn, b);
        if (n->type == AST_BLOCK) {
            ctx->current_ast_block = n;
            // set current symbol table
            rir_ctx_push_st(ctx, ast_block_symbol_table_get((struct ast_node*)n));
            // create allocas for block's symbols and populate the symbol table with rir objects
            rir_ctx_st_create_and_add_allocas(ctx);
            // for each expression of the block create a rir expression and add it to the block
            rf_ilist_for_each(&n->children, child, lh) {
                if (!rir_process_ast_node(child, ctx)) {
                    return false;
                }
            }
            rir_ctx_pop_st(ctx);
        } else if (n->type == AST_MATCH_EXPRESSION) {
            // process match expression as body
            return rir_process_matchexpr((struct ast_node*)n, ctx);
        } else {
            RF_CRITICAL_FAIL("Should never get here");
        }
    }

    return true;
}

struct rir_object *rir_block_create_obj(const struct ast_node *n,
                                        bool function_beginning,
                                        struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, ctx->rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_block_init(ret, n, function_beginning, ctx)) {
        RF_ERROR("Failed to initialize a rir block");
        rir_object_listrem_destroy(ret, ctx);
        ret = NULL;
    }
    return ret;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   bool function_beginning,
                                   struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_create_obj(n, function_beginning, ctx);
    return obj ? &obj->block : NULL;
}

void rir_block_deinit(struct rir_block* b)
{
    // block expressions are part of global oject list and will be destroyed there
    rir_block_exit_deinit(&b->exit);
    rir_value_deinit(&b->label);
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
    if (b->label.category == RIR_VALUE_LABEL) {
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

const struct RFstring *rir_block_label_str(const struct rir_block *b)
{
    if (b->label.category == RIR_VALUE_LABEL) {
        return rir_value_string(&b->label);
    }
    return RFS("");
}

i_INLINE_INS bool rir_block_exit_initialized(const struct rir_block *b);

bool rir_block_is_first(const struct rir_block *b)
{
    return rf_string_equal(&b->label.id, &g_str_fnstart);
}
