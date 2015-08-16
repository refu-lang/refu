#ifndef LFR_IR_RIR_BLOCK_H
#define LFR_IR_RIR_BLOCK_H

#include <RFintrusive_list.h>
#include <ir/rir_branch.h>
#include <ir/rir_strmap.h>

struct ast_node;
struct rir_ctx;
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
    //! Stringmap from normal language identifiers to rir objects
    struct rirexpr_strmap map;
    //! List of rir expressions
    struct RFilist_head expressions;
};

struct rir_block *rir_block_create(const struct ast_node *n,
                                   unsigned int index,
                                   struct rir_ctx *ctx);

/**
 * Destroy this block and all blocks this connects to
 */
void rir_block_destroy(struct rir_block* b);

struct rir_expression *rir_process_ast_node(struct rir_block *b,
                                            const struct ast_node *n,
                                            struct rir_ctx *ctx);

bool rir_block_tostring(struct rir *r, const struct rir_block *b);
#endif
