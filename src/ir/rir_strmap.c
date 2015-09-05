#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

bool rir_fnmap_addobj(struct rir_ctx *ctx,
                      const struct RFstring *id,
                      struct rir_object *obj)
{
    return strmap_add(&ctx->current_fn->map, id, obj);
}

bool rir_map_addobj(struct rir_ctx *ctx,
                    const struct RFstring *id,
                    struct rir_object *obj)
{
    return strmap_add(&ctx->rir->map, id, obj);
}

struct rir_object *rir_fnmap_getobj(struct rir_ctx *ctx,
                                    const struct RFstring *id)
{
    return strmap_get(&ctx->current_fn->map, id);
}

struct rir_expression *rir_fnmap_get_returnslot(struct rir_ctx *ctx)
{
    const struct RFstring returnval_str = RF_STRING_STATIC_INIT("$returnval");
    struct rir_object *ret_slot = rir_fnmap_getobj(ctx, &returnval_str);
    if (!ret_slot || ret_slot->category != RIR_OBJ_EXPRESSION) {
        return NULL;
    }
    return &ret_slot->expr;
}
