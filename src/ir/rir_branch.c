#include <ir/rir_branch.h>

#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>

bool rir_branch_init(struct rir_branch *b, struct rir_expression *dst)
{
    b->dst = dst;
    return true;
}

struct rir_branch *rir_branch_create(struct rir_expression *dst)
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
    // TODO: Remove if not needed
    (void)b;
}

void rir_branch_destroy(struct rir_branch *b)
{
    rir_branch_deinit(b);
    free(b);
}

bool rir_branch_tostring(struct rirtostr_ctx *ctx, const struct rir_branch *b)
{
    bool ret;
    // TODO: remove me: This is temporary due to unimplemented match case generating empty labels
    if (!b->dst) {
        rf_stringx_append_cstr(ctx->rir->buff, "branch(EMPTY_LABEL_FIX_ME)\n");
        return true;
    }

    RFS_PUSH();
    ret = rf_stringx_append(
        ctx->rir->buff,
        RFS("branch("RF_STR_PF_FMT")\n",
            RF_STR_PF_ARG(rir_value_string(&b->dst->val))
        ));
    RFS_POP();
    return ret;
}

bool rir_condbranch_init(struct rir_condbranch *b,
                         struct rir_expression *cond,
                         struct rir_expression *taken,
                         struct rir_expression *fallthrough)
{
    b->cond = cond;
    b->taken = taken;
    b->fallthrough = fallthrough;
    return true;
}

struct rir_condbranch *rir_condbranch_create(struct rir_expression *cond,
                                             struct rir_expression *taken,
                                             struct rir_expression *fallthrough)
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
}

void rir_condbranch_destroy(struct rir_condbranch *b)
{
    rir_condbranch_deinit(b);
    free(b);
}

void rir_condbranch_set_fallthrough(struct rir_condbranch *b,
                                    struct rir_expression *fallthrough)
{
    b->fallthrough = fallthrough;
}

bool rir_condbranch_tostring(struct rirtostr_ctx *ctx, const struct rir_condbranch *b)
{
    RFS_PUSH();
    bool ret = rf_stringx_append(
        ctx->rir->buff,
        RFS("condbranch("RF_STR_PF_FMT", "RF_STR_PF_FMT", "RF_STR_PF_FMT")\n",
            RF_STR_PF_ARG(rir_value_string(&b->cond->val)),
            RF_STR_PF_ARG(rir_value_string(&b->taken->val)),
            RF_STR_PF_ARG(rir_value_string(&b->fallthrough->val))
        ));
    RFS_POP();
    return ret;
}
