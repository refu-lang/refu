#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <ir/rir_value.h>
#include <ir/rir_function.h>
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
                                              struct rir_block *branch_dst)
{
    exit->type = RIR_BLOCK_EXIT_BRANCH;
    return rir_branch_init(&exit->branch, branch_dst);
}

static inline bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                                  struct rir_expression *cond,
                                                  struct rir_block *taken,
                                                  struct rir_block *fallthrough)
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


/* -- functions for rir_block -- */
static struct rir_expression *rir_process_ifexpr(const struct ast_node *n,
                                             unsigned int index,
                                             struct rir_ctx *ctx)
{
#if 0 // TODO, needs thinking. Don't like the index variable.
    struct rir_expression *cond = rir_expression_create(ast_ifexpr_condition_get(n), ctx);
    if (!cond) {
        return false;
    }
    struct rir_block *taken = rir_block_create(ast_ifexpr_taken_block_get(n), 0, ctx);
    if (!taken) {
        return false;
    }
    struct ast_node *fallthrough_branch = ast_ifexpr_fallthrough_branch_get(n);
    // at this point the basic block splits. We neeed a new basic block for the rest
    struct rir_block *new_block = rir_block_create(n, index, ctx);
    if (!new_block) {
        return false;
    }
    return rir_block_exit_init_condbranch(&b->exit, cond, taken, new_block);
#else
    return NULL;
#endif
}

static struct rir_expression *rir_process_vardecl(const struct ast_node *n,
                                                  struct rir_ctx *ctx)
{
#if 0
    struct ast_node *left = ast_types_left(ast_vardecl_desc_get(n));
    const struct RFstring *s = ast_identifier_str(left);
    struct rir_expression *alloca = rir_alloca_create(
        type_get_rir_or_die(ast_node_get_type_or_die(left, AST_TYPERETR_DEFAULT)),
        1,
        ctx
    );
    if (!alloca) {
        return NULL;
    }

    if (!rir_strmap_add_from_id(ctx, s, alloca)) {
        return NULL;
    }
    return alloca;
#else
    return NULL;
#endif
}

static struct rir_expression *rir_process_binaryop(const struct ast_node *n,
                                               struct rir_ctx *ctx)
{
    struct rir_expression *e;
    struct rir_expression *lexpr = rir_process_ast_node(ast_binaryop_left(n), ctx);
    struct rir_expression *rexpr = rir_process_ast_node(ast_binaryop_right(n), ctx);

    switch(ast_binaryop_op(n)) {
    case BINARYOP_ADD:
        e = rir_binaryop_create(RIR_EXPRESSION_ADD, &lexpr->val, &rexpr->val, ctx);
        break;
    default:
        RF_ASSERT(false, "Illegal binary op type detected at rir processing");
        break;
    }
    rirctx_block_add(ctx, e);
    return e;
}

static struct rir_expression *rir_process_return(const struct ast_node *n,
                                                 struct rir_ctx *ctx)
{
    struct rir_expression *ret_val = rir_process_ast_node(ast_returnstmt_expr_get(n), ctx);
    struct rir_expression *ret_expr = rir_return_create(ret_val, ctx);
    // add it to the block
    rirctx_block_add(ctx, ret_expr);
    return ret_expr;
}

static struct rir_expression *rir_process_constant(const struct ast_node *n,
                                                   struct rir_ctx *ctx)
{
    struct rir_expression *ret_expr = rir_constant_create(n, ctx);
    return ret_expr;
}

static struct rir_expression *rir_process_identifier(const struct ast_node *n,
                                                     struct rir_ctx *ctx)
{
    struct rir_expression *expr = strmap_get(&ctx->current_fn->id_map, ast_identifier_str(n));
    if (!expr) {
        RF_ERROR("An identifier was not found in the strmap during rir creation");
        return NULL;
    }
    return expr;
}

// TODO: Delete me. Just a placeholder
struct rir_expression *rir_placeholder_expression(struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->type = RIR_EXPRESSION_PLACEHOLDER;
    // add it to the block
    rirctx_block_add(ctx, ret);
    return ret;
}

struct rir_expression *rir_process_ast_node(const struct ast_node *n,
                                            struct rir_ctx *ctx)
{
    switch (n->type) {
    case AST_IF_EXPRESSION:
        return rir_process_ifexpr(n, /* TODO */0, ctx);
    case AST_VARIABLE_DECLARATION:
        return rir_process_vardecl(n, ctx);
    case AST_BINARY_OPERATOR:
        return rir_process_binaryop(n, ctx);
    case AST_RETURN_STATEMENT:
        return rir_process_return(n, ctx);
    case AST_CONSTANT:
        return rir_process_constant(n, ctx);
    case AST_IDENTIFIER:
        return rir_process_identifier(n, ctx);
    case AST_MATCH_EXPRESSION:
    case AST_MATCH_CASE:
        //TODO: Implement properly, no placeholder
        return rir_placeholder_expression(ctx);
    default:
        RF_ASSERT(false, "Not yet implemented expression for RIR");
    }
    return NULL;
}

static bool rir_block_init(struct rir_block *b,
                           const struct ast_node *n,
                           unsigned int index,
                           struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(b);
    rf_ilist_head_init(&b->expressions);
    struct ast_node *child;
    struct rir_expression *expr;
    unsigned int i = 0;
    ctx->current_block = b;
    // for each expression of the block create a rir expression and add it to the block
    rf_ilist_for_each(&n->children, child, lh) {
        // TODO: pretty stupid way to resume search from a specific index.
        // Rethink this. Maybe linked list is not a good idea here?
        if (i >= index) {
            if (!(expr = rir_process_ast_node(child, ctx))) {
                return false;
            }
        }
    }
    return true;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   struct rir_ctx *ctx)
{
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_block_init(ret, n, index, ctx)) {
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

bool rir_block_tostring(struct rir *r, const struct rir_block *b)
{
    static const struct RFstring open_curly = RF_STRING_STATIC_INIT("\n{\n");
    static const struct RFstring close_curly = RF_STRING_STATIC_INIT("\n}\n");
    struct rir_expression *expr;
    if (!rf_stringx_append(r->buff, &open_curly)) {
        return false;
    }
    rf_ilist_for_each(&b->expressions, expr, ln) {
        if (!rir_expression_tostring(r, expr)) {
            return false;
        }
    }

    if (!rf_stringx_append(r->buff, &close_curly)) {
        return false;
    }
    return true;
}
