#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

bool rir_map_addobj(struct rir_common *c,
                    const struct RFstring *id,
                    struct rir_object *obj)
{
    bool ret = strmap_add(rir_common_curr_map(c), id, obj);
    RF_ASSERT(ret || errno != EEXIST, "Tried to add an already existing RIR object to the map");
    return ret;
}


struct rir_object *rir_map_getobj(struct rir_common *c,
                                  const struct RFstring *id)
{
    struct rir_object *ret = strmap_get(rir_common_curr_map(c), id);
    if (!ret) {
        ret = strmap_get(&c->rir->map, id);
    }
    return ret;
}

struct rirobj_strmap *rir_common_curr_map(struct rir_common *c)
{
    return c->current_fn ? &c->current_fn->map : &c->rir->map;
}
