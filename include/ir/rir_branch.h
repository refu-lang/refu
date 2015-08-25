#ifndef LFR_IR_RIR_BRANCH_H
#define LFR_IR_RIR_BRANCH_H

#include <stdbool.h>

struct rir;
struct rir_block;
struct rir_expression;

struct rir_branch {
    struct rir_expression *dst;
};

bool rir_branch_init(struct rir_branch *b, struct rir_expression *dst);
struct rir_branch *rir_branch_create(struct rir_expression *dst);

void rir_branch_deinit(struct rir_branch *b);
void rir_branch_destroy(struct rir_branch *b);

bool rir_branch_tostring(struct rir *r, const struct rir_branch *b);

struct rir_condbranch {
    struct rir_expression *cond;
    struct rir_expression *taken;
    struct rir_expression *fallthrough;
};

bool rir_condbranch_init(struct rir_condbranch *b,
                         struct rir_expression *cond,
                         struct rir_expression *taken,
                         struct rir_expression *fallthrough);
struct rir_condbranch *rir_condbranch_create(struct rir_expression *cond,
                                             struct rir_expression *taken,
                                             struct rir_expression *fallthrough);
void rir_condbranch_deinit(struct rir_condbranch *b);
void rir_condbranch_destroy(struct rir_condbranch *b);
void rir_condbranch_set_fallthrough(struct rir_condbranch *b,
                                    struct rir_expression *fallthrough);
bool rir_condbranch_tostring(struct rir *r, const struct rir_condbranch *b);
#endif
