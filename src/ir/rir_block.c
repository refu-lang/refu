#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
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
#include <ast/matchexpr.h>
#include <ast/vardecl.h>
#include <ast/block.h>
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
                                    struct rir_value *cond,
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

static struct rir_block *rir_process_elif(const struct ast_node *ast_cond,
                                          const struct ast_node *ast_taken,
                                          const struct ast_node *ast_fallthrough,
                                          struct rir_ctx *ctx);

/* -- functions for rir_block -- */
static struct rir_block *rir_process_conditional(struct rir_value *cond,
                                                 const struct ast_node *ast_taken_block,
                                                 const struct ast_node *ast_fallthrough_branch,
                                                 struct rir_block *old_block,
                                                 struct rir_ctx *ctx)
{
    struct rir_block *taken = rir_block_create(ast_taken_block, false, ctx);
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
            else_block = rir_block_create(ast_fallthrough_branch, false, ctx);
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
    if (!rir_process_ast_node(ast_cond, ctx)) {
        return NULL;
    }
    struct rir_value *cond = rir_ctx_lastval_get(ctx);
    if (!cond) {
        RF_ERROR("A condition value must have been created");
        return NULL;
    }
    return rir_process_conditional(cond, ast_taken, ast_fallthrough, old_block, ctx);
}

static struct rir_block *rir_process_elif(const struct ast_node *ast_cond,
                                          const struct ast_node *ast_taken,
                                          const struct ast_node *ast_fallthrough,
                                          struct rir_ctx *ctx)
{
    struct rir_block *old_block;
    if (!(old_block = rir_block_create(NULL, false, ctx))) {
        return NULL;
    }
    rir_fndecl_add_block(ctx->current_fn, old_block);
    return rir_process_conditional_ast(ast_cond, ast_taken, ast_fallthrough, old_block, ctx);
}
static bool rir_process_ifexpr(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_block *old_block = ctx->current_block;
    RF_ASSERT(!ctx->next_block, "next block should be empty here");
    if (!(ctx->next_block = rir_block_create(NULL, false, ctx))) {
        goto fail;
    }
    ctx->current_block = old_block;

    if (!rir_process_conditional_ast(ast_ifexpr_condition_get(n),
                                     ast_ifexpr_taken_block_get(n),
                                     ast_ifexpr_fallthrough_branch_get(n),
                                     old_block,
                                     ctx)) {
        goto fail;
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
    // Just return the value from the symbol table
    struct ast_node *left = ast_types_left(ast_vardecl_desc_get(n));
    const struct RFstring *id = ast_identifier_str(left);
    struct rir_object *varobj = rir_ctx_st_getobj(ctx, id);
    if (!varobj) {
        RF_ERROR("Could not find a vardecl's RIR object in the symbol table");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, varobj);
}

static struct rir_block *rir_process_matchcase(const struct ast_node *mexpr,
                                               struct rir_value *uni_idx,
                                               struct ast_matchexpr_it *it,
                                               struct rir_block *before_block,
                                               struct rir_block *after_block,
                                               struct ast_node *mcase,
                                               struct rir_ctx *ctx)
{
    struct rir_expression *cmp = NULL;
    struct rir_block *this_block = ctx->current_block;
    bool need_case_cmp = !ast_match_expr_next_case_is_last(mexpr, it) || this_block == before_block;
    if (need_case_cmp) {
        if (this_block != before_block) {
            //create new empty block for the comparisons
            this_block = rir_block_create(NULL, false, ctx);
        }
        // Create index comparison for match case
        struct rir_value *case_idx = rir_constantval_fromint(ast_matchcase_index_get(mcase));
        cmp = rir_binaryop_create_nonast(
            RIR_EXPRESSION_CMP,
            uni_idx,
            case_idx,
            ctx
        );
        rirctx_block_add(ctx, cmp);
    }

    // use this match case symbol table now
    rir_ctx_push_st(ctx, ast_matchcase_symbol_table_get(mcase));
    // create allocas for symbols of this st
    rir_ctx_st_create_allocas(ctx);
    struct ast_node *case_expr = ast_matchcase_expression(mcase);
    struct rir_block *taken = rir_block_create(case_expr, false, ctx);
    if (!taken) {
        return NULL;
    }
    // add the allocas of the symbols to the taken block (should be the current)
    rir_ctx_st_add_allocas(ctx);

    // if there is an assignment to a match expression
    if (ctx->last_assign_obj) {
        struct rir_expression *e = rir_binaryop_create_nonast(
            RIR_EXPRESSION_WRITE,
            rir_object_value(ctx->last_assign_obj),
            rir_ctx_lastval_get(ctx),
            ctx
        );
        if (!e) {
            return NULL;
        }
        rirctx_block_add(ctx, e);
    }

    // stop using this match case symbol table
    rir_ctx_pop_st(ctx);

    if (!rir_block_exit_init_branch(&taken->exit, &after_block->label)) {
        return NULL;
    }

    //try to get next case
    struct ast_node *next_case = ast_matchexpr_next_case(mexpr, it);
    if (next_case) {
        struct rir_block *next_case_block = rir_process_matchcase(mexpr, uni_idx, it, before_block, after_block, next_case, ctx);
        if (need_case_cmp) {
            if (!rir_block_exit_init_condbranch(&this_block->exit, &cmp->val, &taken->label, &next_case_block->label)) {
                return NULL;
            }
        }
    } else {
        // last taken needs to also connect to the after
        // TODO: Last taken should also have a conditional. Maybe only with a specific compiler argument
        // we should check if the sum type actually matched anything and if not terminate the program
        if (!rir_block_exit_init_branch(&taken->exit, &after_block->label)) {
            return NULL;
        }
    }
    return taken;
}

static bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx)
{
    // first of all make sure that all indices to matched types are known
    if (!ast_matchexpr_cases_indices_set(n)) {
        goto fail;
    }

    // find the rir object we are matching
    struct rir_object *matched_obj;
    if (ast_matchexpr_has_header(n)) {
        // if it's a normal match expression
        const struct RFstring *matched_value_str = ast_matchexpr_matched_value_str(n);
        if (!(matched_obj = rir_ctx_st_getobj(ctx, matched_value_str))) {
            RF_ERROR("Match expression identifier was not found in the strmap during rir creation");
            goto fail;
        }
    } else {
        // if it's a match expression acting as function body
        // TODO
        return true;
    }

    // create the after  block
    struct rir_block *prev_block = ctx->current_block;
    struct rir_block *after_block = rir_block_create(NULL, false, ctx);
    if (!after_block) {
        goto fail;
    }
    ctx->current_block = prev_block;

    struct rir_expression *uni_idx = rir_getunionidx_create(rir_object_value(matched_obj), ctx);
    rirctx_block_add(ctx, uni_idx);
    struct ast_matchexpr_it it;
    struct ast_node *mcase = ast_matchexpr_first_case(n, &it);
    struct rir_block *first_case_block = rir_process_matchcase(n,
                                                               &uni_idx->val,
                                                               &it,
                                                               ctx->current_block,
                                                               after_block,
                                                               mcase,
                                                               ctx);
    if (!first_case_block) {
        goto fail;
    }

    ctx->current_block = after_block;
    // since after_block was an empty block let's add it to the function now
    rir_fndecl_add_block(ctx->current_fn, after_block);
    RIRCTX_RETURN_EXPR(ctx, true, NULL);
fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

static bool rir_process_return(const struct ast_node *n,
                               struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(ast_returnstmt_expr_get(n), ctx)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_value *ret_val = rir_ctx_lastval_get(ctx);
    // write the return value to the return slot
    struct rir_expression *ret_slot = rir_fnmap_get_returnslot(ctx);
    if (!ret_slot) {
        RF_ERROR("Could not find the returnvalue of a function in the string map");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *e = rir_binaryop_create_nonast(RIR_EXPRESSION_WRITE, &ret_slot->val, ret_val, ctx);
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
    struct rir_object *ret_expr = rir_constant_create_obj(n, ctx);
    RIRCTX_RETURN_EXPR(ctx, true, ret_expr);
}

bool rir_process_identifier(const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_ctx_st_getobj(ctx, ast_identifier_str(n));
    if (!obj) {
        RF_ERROR("An identifier was not found in the strmap during rir creation");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, obj);
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
        return rir_process_matchexpr((struct ast_node*)n, ctx);
    case AST_CONDITIONAL_BRANCH:
    case AST_MATCH_CASE:
        // Do nothing in these cases
        return true;
    default:
        RF_CRITICAL_FAIL("Not yet implemented expression for RIR");
    }
    return false;
}

struct rir_object *rir_block_functionend_create_obj(bool has_return, struct rir_ctx *ctx)
{
    const struct RFstring fend_label = RF_STRING_STATIC_INIT("%function_end");
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
    struct rir_expression *ret_slot = NULL;
    if (has_return) {
        ret_slot = rir_fnmap_get_returnslot(ctx);
        if (!ret_slot) {
            RF_ERROR("Could not find the returnvalue of a function in the string map");
            RIRCTX_RETURN_EXPR(ctx, false, NULL);
        }
    }
    rir_block_exit_return_init(&ret->block.exit, ret_slot);
    return ret;
}

struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_functionend_create_obj(has_return, ctx);
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
    if (!function_beginning) {
        if (!rir_value_label_init(&b->label, obj, ctx)) {
            return false;
        }
    } else {
        rir_value_nil_init(&b->label);
    }

    struct ast_node *child;
    if (n) {
        // add basic block to the current function
        rir_fndecl_add_block(ctx->current_fn, b);
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
            // for now ignore match expression as body
            return true;
		} else {
			// other expressions, process only one
			if (!rir_process_ast_node(n, ctx)) {
				return false;
			}
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
        goto fail;
    }
    if (!rir_block_init(ret, n, function_beginning, ctx)) {
        goto fail;
    }
    return ret;

fail:
    free(ret);
    return NULL;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   bool function_beginning,
                                   struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_create_obj(n, function_beginning, ctx);
    return obj ? &obj->block : NULL;
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

i_INLINE_INS bool rir_block_exit_initialized(const struct rir_block *b);
