#include <ir/rir_branch.h>

#include <Utils/memory.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>

bool rir_branch_init(struct rir_branch *b, struct rir_block *dst)
{
    b->dst = dst;
    return true;
}

struct rir_branch *rir_branch_create(struct rir_block *dst)
{
    struct rir_branch *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_branch_init(ret, dst)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_branch_deinit(struct rir_branch *b)
{
    rir_block_destroy(b->dst);
}

void rir_branch_destroy(struct rir_branch *b)
{
    rir_branch_deinit(b);
    free(b);
}

bool rir_condbranch_init(struct rir_condbranch *b,
                         struct rir_expression *cond,
                         struct rir_block *taken,
                         struct rir_block *fallthrough)
{
    b->cond = cond;
    b->taken = taken;
    b->fallthrough = fallthrough;
    return true;
}

struct rir_condbranch *rir_condbranch_create(struct rir_expression *cond,
                                             struct rir_block *taken,
                                             struct rir_block *fallthrough)
{
    struct rir_condbranch *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_condbranch_init(ret, cond, taken, fallthrough)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_condbranch_deinit(struct rir_condbranch *b)
{
    rir_expression_destroy(b->cond);
    rir_block_destroy(b->taken);
    rir_block_destroy(b->fallthrough);
}
void rir_condbranch_destroy(struct rir_condbranch *b)
{
    rir_condbranch_deinit(b);
    free(b);
}

void rir_condbranch_set_fallthrough(struct rir_condbranch *b,
                                    struct rir_block *fallthrough)
{
    b->fallthrough = fallthrough;
}
