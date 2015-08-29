#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <Data_Structures/strmap.h>

struct rir_expression;
struct symbol_table_record;
struct rir_ctx;
struct rir_block;

struct rir_symbol {
    struct RFstring *id;
    struct rir_expression *e;
};

struct rirobj_strmap {
    STRMAP_MEMBERS(void*);
};

/**
 * Add a rir object(expression) to the current rir strmap
 *
 * @param ctx            The rir_ctx containing the string map
 * @param id             The string with which to add the object
 * @param e              The rir expression to add
 * @return               false if we run out of memory (errno = ENOMEM), or
 *                       (more normally) if that string already appears in
 *                       the map (EEXIST).
 */
bool rir_strmap_addexpr_from_id(struct rir_ctx *ctx,
                                const struct RFstring *id,
                                struct rir_expression *e);

/**
 * Add a rir object(block) to the current rir strmap
 *
 * @param ctx            The rir_ctx containing the string map
 * @param id             The string with which to add the object
 * @param e              The rir block to add
 * @return               false if we run out of memory (errno = ENOMEM), or
 *                       (more normally) if that string already appears in
 *                       the map (EEXIST).
 */
bool rir_strmap_addblock_from_id(struct rir_ctx *ctx,
                                const struct RFstring *id,
                                struct rir_block *b);
#endif
