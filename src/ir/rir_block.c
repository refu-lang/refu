#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir_expression.h>
#include <ast/ast.h>
#include <ast/ifexpr.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/operators.h>
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

static struct rir_expression *rir_process_ifexpr(struct rir_block *b,
                                                 const struct ast_node *n,
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

static struct rir_expression *rir_process_vardecl(struct rir_block *b,
                                                  const struct ast_node *n,
                                                  struct rir_ctx *ctx)
{
    struct ast_node *left = ast_types_left(ast_vardecl_desc_get(n));
    const struct RFstring *s = ast_identifier_str(left);
    struct rir_expression *alloca = rir_alloca_create(
        type_get_rir_or_die(ast_node_get_type_or_die(left, AST_TYPERETR_DEFAULT)),
        1
    );
    if (!alloca) {
        return NULL;
    }

    if (!rir_strmaps_add_from_id(ctx, s, alloca)) {
        return NULL;
    }
    // add it to the block
    rf_ilist_add(&b->expressions, &alloca->ln);
    return alloca;
}

static struct rir_expression *rir_process_binaryop(struct rir_block *b,
                                                   const struct ast_node *n,
                                                   struct rir_ctx *ctx)
{
    struct rir_expression *e;
    struct rir_expression *lrexpr = rir_process_ast_node(b, ast_binaryop_left(n), ctx);
    struct rir_expression *rrexpr = rir_process_ast_node(b, ast_binaryop_right(n), ctx);

    switch(ast_binaryop_op(n)) {
    case BINARYOP_ADD:
        e = rir_binaryop_create(RIR_EXPRESSION_ADD, lrexpr, rrexpr);
        break;
    default:
        RF_ASSERT(false, "Illegal binary op type detected at rir processing");
        break;
    }
    return e;
}


struct rir_expression *rir_process_ast_node(struct rir_block *b,
                                            const struct ast_node *n,
                                            struct rir_ctx *ctx)
{
    switch (n->type) {
    case AST_IF_EXPRESSION:
        return rir_process_ifexpr(b, n, /* TODO */0, ctx);
    case AST_VARIABLE_DECLARATION:
        return rir_process_vardecl(b, n, ctx);
    case AST_BINARY_OPERATOR:
        return rir_process_binaryop(b, n, ctx);
        break;
    default:
        // TODO
        return NULL;
    }
}

static bool rir_block_init(struct rir_block *b,
                           const struct ast_node *n,
                           unsigned int index,
                           struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(b);
    strmap_init(&b->map);
    struct ast_node *child;
    unsigned int i = 0;
    // for each expression of the block create a rir expression and add it to the block
    rf_ilist_for_each(&n->children, child, lh) {
        // TODO: pretty stupid way to resume search from a specific index.
        // Rethink this. Maybe linked list is not a good idea here?
        if (i >= index) {
            if (!rir_process_ast_node(b, child, ctx)) {
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
    strmap_clear(&b->map);
}

void rir_block_destroy(struct rir_block* b)
{
    rir_block_deinit(b);
    free(b);
}
