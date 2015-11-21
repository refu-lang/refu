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
                                struct rir_value *branch_dst);
bool rir_block_exit_init_condbranch(struct rir_block_exit *exit,
                                    const struct rir_value *cond,
                                    struct rir_value *taken,
                                    struct rir_value *fallthrough);
void rir_block_exit_return_init(struct rir_block_exit *exit,
                                const struct rir_expression *val);

struct rir_block {
    struct rir_block_exit exit;
    //! The block's label value
    struct rir_value label;
    //! List of rir expressions
    struct RFilist_head expressions;
    //! A pointer to the symbol table of the block
    struct symbol_table *st;
};

/**
 * Create a new rir basic block
 *
 * @param n               If given the basic block will start being populated
 *                        with expressions taken out of this ast block. If not,
 *                        a new empty basic block will be created. Additionally
 *                        if this is given then the basic block will be added
 *                        to the function's basic blocks. If not it's left up
 *                        to the caller to add it himself.
 * @param function_start  If true, then this is the first block of a function
 * @param ctx             The rir ctx for traversal
 */
struct rir_object *rir_block_create_obj(const struct ast_node *n,
                                    bool function_beginning,
                                    struct rir_ctx *ctx);
struct rir_block *rir_block_create(const struct ast_node *n,
                                   bool function_beginning,
                                   struct rir_ctx *ctx);
struct rir_block *rir_block_functionend_create(bool has_return, struct rir_ctx *ctx);
struct rir_block *rir_block_matchcase_create(const struct ast_node *mcase,
                                             struct rir_object *matched_rir_obj,
                                             struct rir_ctx *ctx);

void rir_block_destroy(struct rir_block* b);
void rir_block_deinit(struct rir_block* b);

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx);
bool rir_process_identifier(const struct ast_node *n,
                            struct rir_ctx *ctx);

bool rir_block_tostring(struct rirtostr_ctx *ctx, const struct rir_block *b);

i_INLINE_DECL bool rir_block_exit_initialized(const struct rir_block *b)
{
    return b->exit.type != RIR_BLOCK_EXIT_INVALID;
}

/**
 * @note: Should be enclosed in RFS_PUSH() and RFS_POP()
 * @return the string for the rir block label or the empty string if there is none
 */
const struct RFstring *rir_block_label_str(const struct rir_block *b);

/**
 * @return True if the block is the first block of a function
 */
bool rir_block_is_first(const struct rir_block *b);
#endif
