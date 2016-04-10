#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <rflib/datastructs/strmap.h>
#include <rflib/defs/inline.h>

struct symbol_table_record;
struct rir_common;
struct rir_block;

struct rirobj_strmap {
    STRMAP_MEMBERS(struct rir_object*);
};

struct rirtdef_strmap {
    STRMAP_MEMBERS(struct rir_typedef*);
};

struct rirtype_strmap {
    STRMAP_MEMBERS(struct rir_type*);
};

/**
 * Add a rir object to a string to rir_object map
 */
bool rirobj_strmap_add(
    struct rirobj_strmap *map,
    const struct RFstring *id,
    struct rir_object *obj
);

/**
 * Add a rir type to to the rirtype_strmap
 *
 * @param map           The map to add the rir type to
 * @param id            The string to associate it with in the map. The string
 *                      is copied inside the map. Reasoning is that strings in
 *                      rir_types are generated on the fly as temporary strings
 *                      and need to be copied.
 * @param t             The type to add to the map
 * @return              true for success and false for failure
 */
bool rirtype_strmap_add(
    struct rirtype_strmap *map,
    const struct RFstring *id,
    struct rir_type *t
);

/**
 * Add a rir object to the current rir function strmap or if we are not
 * in a function to the global rir map
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
struct rir_type *rirtype_strmap_get(struct rirtype_strmap *map, const struct RFstring *id);

struct rir_value *rir_map_getobj_value(struct rir_common *c,
                                       const struct RFstring *id);

struct rirobj_strmap *rir_common_curr_map(struct rir_common *c);

/**
 * Frees all rir_objects of a rirobj string map and then frees the map itself
 */
void rirobjmap_free(struct rirobj_strmap *map, struct rir_common *c);
/**
 * Frees the rir type strmap and all the strings that are saved in it
 */
void rirtype_strmap_free(struct rirtype_strmap *map);

#ifdef RF_OPTION_DEBUG
/**
 * Print a rir object map for debugging purposes
 */
void rirobjmap_print(const struct rirobj_strmap *m);
#endif
#endif
