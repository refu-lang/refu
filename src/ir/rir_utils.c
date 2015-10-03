#include <ir/rir_utils.h>
#include <ir/rir.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>
#include <ir/rir_expression.h>

#include <Utils/log.h>

struct rir_value g_rir_const_1;
struct rir_value g_rir_const_m1;
static bool utils_created = false;

bool rir_utils_create()
{
    if (!rir_constantval_init_fromint(&g_rir_const_1, 1)) {
        return false;
    }
    if (!rir_constantval_init_fromint(&g_rir_const_m1, -1)) {
        return false;
    }
    utils_created = true;
    return true;
}

void rir_utils_destroy()
{
    if (utils_created) {
        rir_value_deinit(&g_rir_const_m1);
        rir_value_deinit(&g_rir_const_1);
    }
    utils_created = false;
}

const struct rir_value *rir_getread_val(const struct rir_expression *e, struct rir_ctx *ctx)
{
    const struct rir_value *ret = NULL;
    // gotta read the memory value from an alloca
    if (e->type == RIR_EXPRESSION_ALLOCA) {
        struct rir_expression *read;
        read = rir_read_create(&e->val, ctx);
        if (!read) {
            RF_ERROR("Failed to create a read RIR instruction");
            return NULL;
        }
        rirctx_block_add(ctx, read);
        ret = &read->val;
    } else {
        ret = &e->val;
    }
    return ret;
}
