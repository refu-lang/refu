#include <ir/rir_block.h>

#include <rflib/string/core.h>
#include <rflib/string/manipulationx.h>

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

static const struct RFstring rir_blockexit_type_strings[] = {
    [RIR_BLOCK_EXIT_INVALID] = RF_STRING_STATIC_INIT("invalid"),
    [RIR_BLOCK_EXIT_BRANCH] = RF_STRING_STATIC_INIT("branch"),
    [RIR_BLOCK_EXIT_CONDBRANCH] = RF_STRING_STATIC_INIT("condbranch"),
    [RIR_BLOCK_EXIT_RETURN] = RF_STRING_STATIC_INIT("return"),
};
const struct RFstring *rir_blockexit_type_str(enum rir_block_exit_type type)
{
    return &rir_blockexit_type_strings[type];
}
i_INLINE_INS const struct RFstring *rir_block_exit_type_str(const struct rir_block_exit *b);

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
        // if we come here during EXIT_INVALID, it means parsing failed
    case RIR_BLOCK_EXIT_RETURN:
        // the return stmt should be a global rir object so is freed from global rir objects list
        break;
    }
}

void rir_block_exit_return_init(struct rir_block_exit *exit,
                                const struct rir_value *val)
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
        if (exitb->retstmt.val) {
            if (!rf_stringx_append(
                    ctx->rir->buff,
                    RFS(RIRTOSTR_INDENT"return("RFS_PF")\n",
                        RFS_PA(rir_value_string(exitb->retstmt.val)))
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
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, ctx->common.rir);
    if (!ret) {
        goto fail_free_ret;
    }
    struct rir_block *b = &ret->block;
    RF_STRUCT_ZERO(b);
    ctx->common.current_block = b;
    rf_ilist_head_init(&b->expressions);
    if (!rir_value_label_init_string(&ret->block.label, ret, &fend_label, &ctx->common)) {
        goto fail_free_ret;
    }

    // current block's exit should be the return
    struct rir_value *return_val = NULL;
    if (has_return) {
        if (rir_type_is_composite(rir_ctx_curr_fn(ctx)->retslot_val->type)) {
            // if returning a user type, return directly
            return_val = rir_ctx_curr_fn(ctx)->retslot_val;
        } else { // else read the value from the pointer
            struct rir_expression *e = rir_read_create(
                rir_ctx_curr_fn(ctx)->retslot_val,
                RIRPOS_AST,
                ctx
            );
            if (!e) {
                RF_ERROR("Could not create a read from a function's return slot");
                goto fail_free_ret;
            }
            rir_common_block_add(&ctx->common, e);
            return_val = &e->val;
        }
    }
    rir_block_exit_return_init(&ret->block.exit, return_val);
    b->st = rir_ctx_curr_st(ctx);
    RF_ASSERT(b->st, "Symbol table should not be NULL");

    return ret;

fail_free_ret:
    free (ret);
    return NULL;
}

struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_block_functionend_create_obj(has_return, ctx);
    return obj ? &obj->block : NULL;
}

static bool rir_block_init(
    struct rir_object *obj,
    const struct RFstring *name,
    enum rir_pos pos,
    rir_data data
)
{
    struct rir_block *b = &obj->block;
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    rir_data_curr_block(data) = b;
    if (!rir_value_label_init_string(&b->label, obj, name, rir_data_common(data))) {
        return false;
    }
    return true;
}

// @warning: wrap in RFS_PUSH() and RFS_POP()
static inline const struct RFstring *rir_block_from_ast_new_labelname(
    bool function_beginning,
    struct rir_ctx *ctx
)
{
   if (function_beginning) {
       return &g_str_fnstart;
   } else {
       return RFS("label_%d", ctx->label_idx++);
   }
}

static inline bool rir_block_init_from_ast_common(
    struct rir_object *obj,
    bool function_beginning,
    struct rir_ctx *ctx
)
{
    bool ret = false;
    RFS_PUSH();
    const struct RFstring *name = rir_block_from_ast_new_labelname(function_beginning, ctx);
    if (!name) {
        goto end;
    }
    if (!rir_block_init(obj, name, RIRPOS_AST, ctx)) {
        goto end;

    }
    // success
    ret = true;
end:
    RFS_POP();
    return ret;
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
static bool rir_block_init_from_ast(
    struct rir_object *obj,
    const struct ast_node *n,
    bool function_beginning,
    struct rir_ctx *ctx
)
{
    struct rir_block *b = &obj->block;
    if (!rir_block_init_from_ast_common(obj, function_beginning, ctx)) {
        return false;
    }

    struct ast_node *child;
    if (n) {
        // add basic block to the current function
        rir_fndef_add_block(rir_ctx_curr_fn(ctx), b);
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
            b->st = rir_ctx_curr_st(ctx);
            rir_ctx_pop_st(ctx);
        } else if (n->type == AST_MATCH_EXPRESSION) {
            // process match expression as body
            b->st = rir_ctx_curr_st(ctx);
            RF_ASSERT(b->st, "Symbol table should not be NULL");
            return rir_process_matchexpr((struct ast_node*)n, ctx);
        } else {
            RF_CRITICAL_FAIL("Should never get here");
            return false;
        }
    } else {
        b->st = rir_ctx_curr_st(ctx);
    }

    // success
    RF_ASSERT(b->st, "Symbol table should not be NULL");
    return true;
}

struct rir_object *rir_block_create_obj_from_ast(
    const struct ast_node *n,
    bool function_beginning,
    struct rir_ctx *ctx
)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, rir_ctx_rir(ctx));
    if (!ret) {
        return NULL;
    }
    if (!rir_block_init_from_ast(ret, n, function_beginning, ctx)) {
        RF_ERROR("Failed to initialize a rir block");
        rir_object_listrem_destroy(ret, rir_ctx_rir(ctx), rir_ctx_curr_fn(ctx));
        ret = NULL;
    }
    return ret;
}

struct rir_block *rir_block_create_from_ast(
    const struct ast_node *n,
    bool function_beginning,
    struct rir_ctx *ctx
)
{
    struct rir_object *obj = rir_block_create_obj_from_ast(n, function_beginning, ctx);
    return obj ? &obj->block : NULL;
}

static struct rir_object *rir_block_matchcase_create_obj(const struct ast_node *mcase,
                                                         struct rir_object *matched_rir_obj,
                                                         struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, rir_ctx_rir(ctx));
    if (!ret) {
        goto fail;
    }
    struct rir_block *b = &ret->block;
    if (!rir_block_init_from_ast_common(ret, false, ctx)) {
        goto fail;
    }

    AST_NODE_ASSERT_TYPE(mcase, AST_MATCH_CASE);
    // add basic block to the current function
    rir_fndef_add_block(rir_ctx_curr_fn(ctx), b);
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
    b->st = mcase->matchcase.st;
    RF_ASSERT(b->st, "Symbol table should not be NULL");
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

struct rir_object *rir_block_create_obj(
    const struct RFstring *name,
    enum rir_pos pos,
    rir_data data
)
{
    RF_ASSERT(pos == RIRPOS_PARSE, "At least for now only through parsing we get here");
    struct rir_object *ret = rir_object_create(RIR_OBJ_BLOCK, rir_data_rir(data));
    if (!ret) {
        return NULL;
    }
    if (!rir_block_init(ret, name, pos, data)) {
        RF_ERROR("Failed to initialize a rir block");
        rir_object_listrem_destroy(ret, rir_data_rir(data), rir_data_curr_fn(data));
        ret = NULL;
    }
    return ret;
}

struct rir_block *rir_block_create(
    const struct RFstring *name,
    enum rir_pos pos,
    rir_data data
)
{
    struct rir_object *obj = rir_block_create_obj(name, pos, data);
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

i_INLINE_INS void rir_block_add_expr(struct rir_block *b, struct rir_expression *e);
