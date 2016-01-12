#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <Data_Structures/strmap.h>
#include <Definitions/inline.h>

struct rir_object;
struct symbol_table_record;
struct rir_common;
struct rir_block;


struct rirobj_strmap {
    STRMAP_MEMBERS(struct rir_object*);
};

struct rirtdef_strmap {
    STRMAP_MEMBERS(struct rir_typedef*);
};

/**
 * Add a rir object to a string to rir_object map
 */
bool rirobj_strmap_add(struct rirobj_strmap *map,
                       const struct RFstring *id,
                       struct rir_object *obj);

/**
 * Add a rir object to the current rir function strmap or if we are not in a function
 * to the global rir map
 *
 * @param common         The common rir data containing the map
 * @param id             The string with which to add the object
 * @param obj            The rir object to add
 * @return               false if we run out of memory (errno = ENOMEM), or
 *                       (more normally) if that string already appears in
 *                       the map (EEXIST).
 */
bool rir_map_addobj(struct rir_common *c,
                    const struct RFstring *id,
                    struct rir_object *obj);


struct rir_object *rir_map_getobj(struct rir_common *c,
                                  const struct RFstring *id);

struct rirobj_strmap *rir_common_curr_map(struct rir_common *c);

/**
 * Frees all rir_objects of a rirobj string map and then frees the map itself
 */
void rirobjmap_free(struct rirobj_strmap *map, struct rir_common *c);
#endif
