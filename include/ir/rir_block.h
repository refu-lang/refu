#ifndef LFR_IR_RIR_BLOCK_H
#define LFR_IR_RIR_BLOCK_H

#include <RFintrusive_list.h>

struct ast_node;

struct rir_block {
    struct rir_block *next_block;
    //! List of rir expressions
    struct RFilist_head expressions;
};

struct rir_block *rir_block_create(const struct ast_node *n);

/**
 * Destroy this block and all blocks this connects to
 */
void rir_block_destroy(struct rir_block* b);
#endif
