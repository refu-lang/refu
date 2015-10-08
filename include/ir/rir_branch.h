#ifndef LFR_IR_RIR_BRANCH_H
#define LFR_IR_RIR_BRANCH_H

#include <stdbool.h>

struct rirtostr_ctx;
struct rir_block;
struct rir_expression;

struct rir_branch {
    struct rir_value *dst;
};

bool rir_branch_init(struct rir_branch *b, struct rir_value *dst);
struct rir_branch *rir_branch_create(struct rir_value *dst);

void rir_branch_deinit(struct rir_branch *b);
void rir_branch_destroy(struct rir_branch *b);

bool rir_branch_tostring(struct rirtostr_ctx *ctx, const struct rir_branch *b);

struct rir_condbranch {
    const struct rir_value *cond;
    struct rir_value *taken;
    struct rir_value *fallthrough;
};

bool rir_condbranch_init(struct rir_condbranch *b,
                         const struct rir_value *cond,
                         struct rir_value *taken,
                         struct rir_value *fallthrough);
struct rir_condbranch *rir_condbranch_create(const struct rir_value *cond,
                                             struct rir_value *taken,
                                             struct rir_value *fallthrough);
void rir_condbranch_deinit(struct rir_condbranch *b);
void rir_condbranch_destroy(struct rir_condbranch *b);
bool rir_condbranch_tostring(struct rirtostr_ctx *ctx, const struct rir_condbranch *b);
#endif
