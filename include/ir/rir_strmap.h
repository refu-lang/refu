#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <Data_Structures/strmap.h>

struct rir_expression;
struct symbol_table_record;
struct rir_ctx;

struct rir_symbol {
    struct RFstring *id;
    struct rir_expression *e;
};

struct rirexpr_strmap {
    STRMAP_MEMBERS(struct rir_expression *);
};

/**
 * This returns false if we run out of memory (errno = ENOMEM), or
 * (more normally) if that string already appears in the map (EEXIST).
 */
bool rir_strmap_add_from_id(struct rir_ctx *ctx,
                            const struct RFstring *id,
                            struct rir_expression *e);
#endif
