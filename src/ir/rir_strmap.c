#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

bool rir_map_addobj(struct rir_ctx *ctx,
                    const struct RFstring *id,
                    struct rir_object *obj)
{
    bool ret;
    struct rirobj_strmap *map = ctx->current_fn ? &ctx->current_fn->map : &ctx->rir->map;
    ret = strmap_add(map, id, obj);
    RF_ASSERT(ret || errno != EEXIST, "Tried to add an already existing RIR object to the map");
    return ret;
}


struct rir_object *rir_map_getobj(struct rir_ctx *ctx,
                                  const struct RFstring *id)
{
    struct rir_object *ret;
    struct rirobj_strmap *map = ctx->current_fn ? &ctx->current_fn->map : &ctx->rir->map;
    ret = strmap_get(map, id);
    if (!ret) {
        ret = strmap_get(&ctx->rir->map, id);
    }
    return ret;
}
