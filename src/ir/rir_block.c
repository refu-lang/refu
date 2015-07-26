#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <ir/rir_expression.h>
#include <ast/ast.h>
#include <ast/ifexpr.h>

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

static bool rir_process_ifexpr(struct rir_block *b,
                               const struct ast_node *n,
                               unsigned int index,
                               struct rir *r)
{
    struct rir_expression *cond = rir_expression_create(ast_ifexpr_condition_get(n), r);
    if (!cond) {
        return false;
    }
    struct rir_block *taken = rir_block_create(ast_ifexpr_taken_block_get(n), 0, r);
    if (!taken) {
        return false;
    }
#if 0 // TODO: implement fallthrough branch
    struct ast_node *fallthrough_branch = ast_ifexpr_fallthrough_branch_get(n);
#endif
    // at this point the basic block splits. We neeed a new basic block for the rest
    struct rir_block *new_block = rir_block_create(n, index, r);
    if (!new_block) {
        return false;
    }
    return rir_block_exit_init_condbranch(&b->exit, cond, taken, new_block);
}

static bool rir_block_init(struct rir_block *b,
                           const struct ast_node *n,
                           unsigned int index,
                           struct rir *r)
{
    RF_STRUCT_ZERO(b);
    struct ast_node *child;
    struct rir_expression *expr;
    unsigned int i = 0;
    // for each expression of the block create a rir expression and add it to the block
    rf_ilist_for_each(&n->children, child, lh) {
        // TODO: pretty stupid way to resume search from a specific index.
        // Rethink this. Maybe linked list is not a good idea here?
        if (i >= index) {
            switch (child->type) {
            case AST_IF_EXPRESSION:
                return rir_process_ifexpr(b, n, i, r);
            default:
                expr = rir_expression_create(child, r);
                if (!expr) {
                    return false;
                }
                rf_ilist_add(&b->expressions, &expr->ln);
                break;
            }
        }
    }
    return true;
}

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   struct rir *r)
{
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_block_init(ret, n, index, r)) {
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
