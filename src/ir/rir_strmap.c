#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

bool rir_strmap_add_from_id(struct rir_ctx *ctx,
                            const struct RFstring *id,
                            struct rir_expression *e)
{
    // add normal language identifier to current block
    if (!strmap_add(&ctx->current_block->map, id, e)) {
        return false;
    }
    return true;
}
