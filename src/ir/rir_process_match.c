#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_constant.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir_type.h>
#include <types/type.h>
#include <ast/matchexpr.h>
#include <Utils/sanity.h>

struct alloca_pop_ctx {
    struct rir_ctx *rirctx;
    const struct rir_type *matched_case_rirtype;
    struct rir_value *matched_case_rirobj;
};

static inline void alloca_pop_ctx_init(struct alloca_pop_ctx *ctx,
                                       struct rir_ctx *rirctx,
                                       const struct rir_type *matched_case_rirtype,
                                       struct rir_value *matched_case_rirobj)
{
    ctx->rirctx = rirctx;
    ctx->matched_case_rirtype = matched_case_rirtype;
    ctx->matched_case_rirobj = matched_case_rirobj;
}

static void rir_symbol_table_populate_allocas_do(struct symbol_table_record *rec,
                                                 struct alloca_pop_ctx *ctx)
{
    RF_ASSERT(rec->data, "Record should have a type");
    const struct rir_type *rtype = type_get_rir_or_die(rec->data);
    RF_ASSERT(rec->rirobj, "Record should have an associated rir object");
    struct rir_value *val;
    int idx = rir_type_childof_type(rtype, ctx->matched_case_rirtype);
    if (idx != -1) { // type child of matched type
        // create a rir expression to read the object value at the index position
        struct rir_object *e = rir_objmemberat_create_obj(ctx->matched_case_rirobj, idx, ctx->rirctx);
        rirctx_block_add(ctx->rirctx, &e->expr);
        val = rir_object_value(e);
    } else if (rtype == ctx->matched_case_rirtype) { // type is actually matched type
        val = ctx->matched_case_rirobj;
    } else {
        RF_CRITICAL_FAIL("Type should either have been found as child of the case type or be equal to it");
    }
    // and now write that to the alloca
    struct rir_expression *expr = rir_binaryop_create_nonast(
        RIR_EXPRESSION_WRITE,
        rir_object_value(rec->rirobj),
        val,
        ctx->rirctx
    );
    rirctx_block_add(ctx->rirctx, expr);
}

static struct rir_block *rir_process_matchcase(const struct ast_node *mexpr,
                                               struct rir_object *matched_rir_obj,
                                               struct rir_value *uni_idx,
                                               struct ast_matchexpr_it *it,
                                               struct rir_block *before_block,
                                               struct rir_block *after_block,
                                               struct ast_node *mcase,
                                               struct rir_ctx *ctx)
{
    struct rir_expression *cmp = NULL;
    struct rir_block *this_block = ctx->current_block;
    uint32_t case_idx = ast_matchcase_index_get(mcase);
    struct rir_value *case_rir_idx = rir_constantval_create_fromint(ast_matchcase_index_get(mcase));
    bool need_case_cmp = !ast_match_expr_next_case_is_last(mexpr, it) || this_block == before_block;
    if (need_case_cmp) {
        if (this_block != before_block) {
            //create new empty block for the comparisons
            this_block = rir_block_create(NULL, false, ctx);
            rir_fndef_add_block(ctx->current_fn, this_block);
        }
        // Create index comparison for match case
        cmp = rir_binaryop_create_nonast(
            RIR_EXPRESSION_CMP,
            uni_idx,
            case_rir_idx,
            ctx
        );
        rirctx_block_add(ctx, cmp);
    }

    // use this match case symbol table now
    rir_ctx_push_st(ctx, ast_matchcase_symbol_table_get(mcase));
    // create allocas for symbols of this st
    rir_ctx_st_create_allocas(ctx);

    // create the rir block for this case
    struct ast_node *case_expr = ast_matchcase_expression(mcase);
    struct rir_block *taken = rir_block_create(case_expr, false, ctx);
    if (!taken) {
        return NULL;
    }
    // Get the type of this match case and read the union's subobject
    const struct type *mcmatch_type = ast_matchcase_matched_type(mcase);
    struct rir_expression *acc_subtype = rir_unionmemberat_create(
        rir_object_value(matched_rir_obj),
        case_idx,
        ctx
    );
    rirctx_block_add(ctx, acc_subtype);
    // now iterate the allocas and assign the proper values from the subobject
    struct alloca_pop_ctx alloca_ctx;
    alloca_pop_ctx_init(&alloca_ctx, ctx, type_get_rir_or_die(mcmatch_type), &acc_subtype->val);
    symbol_table_iterate(rir_ctx_curr_st(ctx),
                         (htable_iter_cb)rir_symbol_table_populate_allocas_do,
                         &alloca_ctx);

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
        struct rir_block *next_case_block = rir_process_matchcase(mexpr, matched_rir_obj, uni_idx, it, before_block, after_block, next_case, ctx);
        if (need_case_cmp) {
            if (!rir_block_exit_init_condbranch(&this_block->exit, &cmp->val, &taken->label, &next_case_block->label)) {
                return NULL;
            }
            return this_block;
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

bool rir_process_matchexpr(struct ast_node *n, struct rir_ctx *ctx)
{
    // first of all make sure that all indices to matched types are known
    if (!ast_matchexpr_cases_indices_set(n)) {
        goto fail;
    }

    // find the rir object we are matching
    struct rir_object *matched_obj;
    struct rir_block *after_block;
    if (ast_matchexpr_has_header(n)) {
        const struct RFstring *matched_value_str = ast_matchexpr_matched_value_str(n);
        if (!(matched_obj = rir_ctx_st_getobj(ctx, matched_value_str))) {
            RF_ERROR("Match expression identifier was not found in the strmap during rir creation");
            goto fail;
        }
        // create the after block
        struct rir_block *prev_block = ctx->current_block;
        after_block = rir_block_create(NULL, false, ctx);
        if (!after_block) {
            goto fail;
        }
        ctx->current_block = prev_block;
    } else {
        struct symbol_table_record *rec = ast_matchexpr_headless_strec(
            n, rir_ctx_curr_st(ctx));
        RF_ASSERT(rec->rirobj, "A rir object should have been set for the record");
        matched_obj = rec->rirobj;
        after_block = rir_value_label_dst(ctx->current_fn->end_label);
    }

    struct rir_expression *uni_idx = rir_getunionidx_create(rir_object_value(matched_obj), ctx);
    rirctx_block_add(ctx, uni_idx);
    struct ast_matchexpr_it it;
    struct ast_node *mcase = ast_matchexpr_first_case(n, &it);
    struct rir_block *first_case_block = rir_process_matchcase(n,
                                                               matched_obj,
                                                               &uni_idx->val,
                                                               &it,
                                                               ctx->current_block,
                                                               after_block,
                                                               mcase,
                                                               ctx);
    if (!first_case_block) {
        goto fail;
    }

    // for normal matchexpr, after_block was an empty block so let's add it to the function now
    if (ast_matchexpr_has_header(n)) {
        ctx->current_block = after_block;
        rir_fndef_add_block(ctx->current_fn, after_block);
    }
    RIRCTX_RETURN_EXPR(ctx, true, NULL);
fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}
