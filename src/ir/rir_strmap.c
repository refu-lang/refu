#include <ir/rir_strmap.h>

#include <rflib/string/core.h>

#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

bool rirobj_strmap_add(
    struct rirobj_strmap *map,
    const struct RFstring *id,
    struct rir_object *obj
)
{
    bool ret = strmap_add(map, (struct RFstring*)id, obj);
    RF_ASSERT(
        ret || errno != EEXIST,
        "Tried to add an already existing RIR object to the map"
    );
    return ret;
}

bool rirtype_strmap_add(
    struct rirtype_strmap *map,
    const struct RFstring *id,
    struct rir_type *t)
{
    struct RFstring *id_copy = rf_string_copy_out(id);
    if (!id_copy) {
        return false;
    }
    bool ret = strmap_add(map, id_copy, t);
    RF_ASSERT(
        ret || errno != EEXIST,
        "Tried to add an already existing RIR object to the map"
    );
    if (!ret) {
        rf_string_destroy(id_copy);
    }
    return ret;
}

struct rir_type *rirtype_strmap_get(struct rirtype_strmap *map, const struct RFstring *id)
{
    return strmap_get(map, id);
}

static bool itfree_rirtypestring(struct RFstring *s, struct rir_type *t, void *u)
{
    (void)t;
    (void)u;
    rf_string_destroy(s);
    return true;
}

void rirtype_strmap_free(struct rirtype_strmap *map)
{
    strmap_iterate(map, (strmap_it_cb)itfree_rirtypestring, NULL);
    strmap_clear(map);
}

bool rir_map_addobj(
    struct rir_common *c,
    const struct RFstring *id,
    struct rir_object *obj
)
{
    return rirobj_strmap_add(rir_common_curr_map(c), id, obj);
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

struct rir_value *rir_map_getobj_value(struct rir_common *c,
                                       const struct RFstring *id)
{
    struct rir_object *obj = rir_map_getobj(c, id);
    return obj ? rir_object_value(obj) : NULL;
}

struct rirobj_strmap *rir_common_curr_map(struct rir_common *c)
{
    return c->current_fn ? &c->current_fn->map : &c->rir->map;
}


static bool itfree_rirobjects(const struct RFstring *s, struct rir_object *obj, struct rir_common *common)
{
    rir_object_listrem_destroy(obj, common->rir, common->current_fn);
    return true;
}

void rirobjmap_free(struct rirobj_strmap *map, struct rir_common *c)
{
    strmap_iterate(map, (strmap_it_cb)itfree_rirobjects, c);
    strmap_clear(map);
}

#ifdef RF_OPTION_DEBUG
static bool rirobjmap_print_cb(const struct RFstring *s, struct rir_object *obj)
{
    printf(
        RFS_PF " -- " RFS_PF "\n",
        RFS_PA(rir_object_string(obj)),
        RFS_PA(s)
    );
    return true;
}
void rirobjmap_print(const struct rirobj_strmap *m)
{
    strmap_iterate(m, (strmap_it_cb)rirobjmap_print_cb, NULL);
}
#endif
