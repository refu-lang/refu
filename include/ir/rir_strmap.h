#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <Data_Structures/strmap.h>

struct rir_object;
struct symbol_table_record;
struct rir_ctx;
struct rir_block;


struct rirobj_strmap {
    STRMAP_MEMBERS(struct rir_object*);
};

struct rirtdef_strmap {
    STRMAP_MEMBERS(struct rir_typedef*);
};

/**
 * Add a rir object to the current rir function strmap or if we are not in a function
 * to the global rir map
 *
 * @param ctx            The rir_ctx containing the string map
 * @param id             The string with which to add the object
 * @param obj            The rir object to add
 * @return               false if we run out of memory (errno = ENOMEM), or
 *                       (more normally) if that string already appears in
 *                       the map (EEXIST).
 */
bool rir_map_addobj(struct rir_ctx *ctx,
                    const struct RFstring *id,
                    struct rir_object *obj);


struct rir_object *rir_map_getobj(struct rir_ctx *ctx,
                                  const struct RFstring *id);
#endif
