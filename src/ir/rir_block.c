#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <ir/rir_value.h>
#include <ir/rir_function.h>
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
static inline bool rir_block_exit_init_branch(struct rir_block_exit *exit,
                                              struct rir_expression *branch_dst)
{
    exit->type = RIR_BLOCK_EXIT_BRANCH;
    return rir_branch_init(&exit->branch, branch_dst);
}

static inline bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                                  struct rir_expression *cond,
                                                  struct rir_expression *taken,
                                                  struct rir_expression *fallthrough)
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

static bool rir_blockexit_tostring(struct rir *r, const struct rir_block_exit *exit)
{
    bool ret = false;
    RFS_PUSH();
    switch (exit->type) {
    case RIR_BLOCK_EXIT_BRANCH:
        return rir_branch_tostring(r, &exit->branch);
    case RIR_BLOCK_EXIT_CONDBRANCH:
        if (!rir_condbranch_tostring(r, &exit->condbranch)) {
            goto end;
        }
        if (!rir_block_tostring(r, exit->condbranch.taken->label.block, exit->condbranch.taken->label.index)) {
            goto end;
        }
        if (exit->condbranch.fallthrough) {
                 if (!rir_block_tostring(r, exit->condbranch.fallthrough->label.block, exit->condbranch.fallthrough->label.index)) {
                     goto end;
                 }
        }
        break;
    case RIR_BLOCK_EXIT_RETURN:
        if (!rf_stringx_append(
                r->buff,
                RFS("return("RF_STR_PF_FMT")\n",
                    RF_STR_PF_ARG(rir_value_string(&exit->retstmt.ret.val->val)))
            )) {
            goto end;
        }
        break;
    }

    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}


/* -- functions for rir_block -- */
static bool rir_process_ifexpr(const struct ast_node *n,
                               unsigned int index,
                               struct rir_ctx *ctx)
{
    struct rir_block *old_block = ctx->current_block;
    if (!rir_process_ast_node(ast_ifexpr_condition_get(n), ctx)) {
        goto fail;
    }
    struct rir_expression *cond = ctx->returned_expr;
    struct rir_block *taken = rir_block_create(ast_ifexpr_taken_block_get(n), 0, false, ctx);
    if (!taken) {
        goto fail;
    }
    // TODO: fallthrough
    /* struct ast_node *fallthrough_branch = ast_ifexpr_fallthrough_branch_get(n); */
    // at this point the basic block splits. We neeed a new basic block for the rest
    struct rir_block *new_block = rir_block_create(n, index, false, ctx);
    if (!new_block) {
        goto fail;
    }
    //Connect the old block with the new
    if (!rir_block_exit_init_condbranch(&old_block->exit, cond, taken->label, new_block->label)) {
        goto fail;
    }
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
    if (!rir_process_ast_node(ast_returnstmt_expr_get(n), ctx)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *ret_val = ctx->returned_expr;

    // current block's exit should be the return
    if (!rir_block_exit_return_init(&ctx->current_block->exit, ret_val, ctx)) {
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

static bool rir_process_identifier(const struct ast_node *n,
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
        return rir_process_ifexpr(n, /* TODO */0, ctx);
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
    case AST_MATCH_EXPRESSION:
        // TODO: implement properly
    case AST_CONDITIONAL_BRANCH:
    case AST_MATCH_CASE:
        // Do nothing in these cases
        return true;
    default:
        RF_ASSERT(false, "Not yet implemented expression for RIR");
    }
    return false;;
}

static bool rir_block_init(struct rir_block *b,
                           const struct ast_node *n,
                           unsigned int index,
                           bool function_beginning,
                           struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    ctx->current_block = b;
    if (!function_beginning) {
        b->label = rir_label_create(b, 0, ctx);
        rirctx_block_add(ctx, b->label);
    }

    struct ast_node *child;
    unsigned int i = 0;
    // for each expression of the block create a rir expression and add it to the block
    rf_ilist_for_each(&n->children, child, lh) {
        // TODO: pretty stupid way to search from a specific index.
        // Rethink this. Maybe linked list is not a good idea here?
        if (i >= index) {
            if (!rir_process_ast_node(child, ctx)) {
                return false;
            }
        }
        ++i;
    }
    return true;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   bool function_beginning,
                                   struct rir_ctx *ctx)
{
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_block_init(ret, n, index, function_beginning, ctx)) {
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

bool rir_block_tostring(struct rir *r, const struct rir_block *b, unsigned index)
{
    struct rir_expression *expr;

    unsigned int i = 0;
    rf_ilist_for_each(&b->expressions, expr, ln) {
        // TODO: pretty stupid way to search from a specific index.
        // Rethink this. Maybe linked list is not a good idea here?
        if (i >= index) {
            if (!rir_expression_tostring(r, expr)) {
                return false;
            }
        }
        ++i;
    }

    if (!rir_blockexit_tostring(r, &b->exit)) {
        return false;
    }

    return true;
}
