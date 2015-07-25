#ifndef LFR_IR_RIR_BLOCK_H
#define LFR_IR_RIR_BLOCK_H

#include <RFintrusive_list.h>
#include <ir/rir_branch.h>

struct ast_node;
struct rir;

enum rir_block_exit_type {
    RIR_BLOCK_EXIT_BRANCH,
    RIR_BLOCK_EXIT_CONDBRANCH,
    RIR_BLOCK_EXIT_RETURN,
};

struct rir_block_exit {
    enum rir_block_exit_type type;
    union {
        struct rir_branch branch;
        struct rir_condbranch condbranch;
    };
};

struct rir_block {
    struct rir_block_exit exit;
    //! List of rir expressions
    struct RFilist_head expressions;
};

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   struct rir *r);

/**
 * Destroy this block and all blocks this connects to
 */
void rir_block_destroy(struct rir_block* b);
#endif
