#ifndef LFR_IR_RIR_BLOCK_H
#define LFR_IR_RIR_BLOCK_H

#include <RFintrusive_list.h>
#include <ir/rir_branch.h>
#include <ir/rir_strmap.h>
#include <ir/rir_expression.h>


struct ast_node;
struct rir_ctx;
struct rir;

enum rir_block_exit_type {
    RIR_BLOCK_EXIT_INVALID = 0,
    RIR_BLOCK_EXIT_BRANCH,
    RIR_BLOCK_EXIT_CONDBRANCH,
    RIR_BLOCK_EXIT_RETURN,
};

struct rir_block_exit {
    enum rir_block_exit_type type;
    union {
        struct rir_expression retstmt;
        struct rir_branch branch;
        struct rir_condbranch condbranch;
    };
};

bool rir_block_exit_init_branch(struct rir_block_exit *exit,
                                struct rir_expression *branch_dst);
bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                    struct rir_expression *cond,
                                    struct rir_expression *taken,
                                    struct rir_expression *fallthrough);
bool rir_block_exit_return_init(struct rir_block_exit *exit,
                                const struct rir_expression *val,
                                struct rir_ctx *ctx);

struct rir_block {
    struct rir_block_exit exit;
    //! The block's label. If NULL, then it's first block of a function
    struct rir_expression *label;
    //! List of rir expressions
    struct RFilist_head expressions;
};

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   bool function_beginning,
                                   struct rir_ctx *ctx);
struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx);

/**
 * Destroy this block and all blocks this connects to
 */
void rir_block_destroy(struct rir_block* b);

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);

bool rir_block_tostring(struct rirtostr_ctx *ctx, const struct rir_block *b, unsigned index);

i_INLINE_DECL bool rir_block_exit_initialized(const struct rir_block *b)
{
    return b->exit.type != RIR_BLOCK_EXIT_INVALID;
}
#endif
