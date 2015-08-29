#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

bool rir_strmap_addexpr_from_id(struct rir_ctx *ctx,
                            const struct RFstring *id,
                            struct rir_expression *e)
{
    return strmap_add(&ctx->current_fn->map, id, e);
}

bool rir_strmap_addblock_from_id(struct rir_ctx *ctx,
                                const struct RFstring *id,
                                struct rir_block *b)
{
    return strmap_add(&ctx->current_fn->map, id, b);
}
