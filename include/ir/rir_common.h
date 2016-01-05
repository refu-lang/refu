#ifndef LFR_IR_RIR_COMMON_H
#define LFR_IR_RIR_COMMON_H


struct rir;
struct rir_fndef;
struct rir_block;
struct rir_expression;

/**
 * Common data required by both ast and parsing rir code
 */
struct rir_common {
    struct rir *rir;
    struct rir_fndef *current_fn;
    struct rir_block *current_block;
};

/**
 * Denotes which module the rir code is called from
 */
enum rir_pos {
    //! Function called directly from AST to RIR conversion 
    RIRPOS_AST = 0,
    //! Function called during RIR file parsing
    RIRPOS_PARSE = 1,
};

/**
 * The rir data. If @a pos:
 *    + RIRPOS_AST   -> then this is rir_ctx
 *    + RIRPOS_PARSE -> then this is rir_pctx
 */
typedef void* rir_data;


void rir_common_block_add(struct rir_common *c, struct rir_expression *expr);


/**
 * Convenience macros to get rir common members out of rir_data
 *
 * @warning Kind of a hack, we rely on rir_common
 * being first member of rir_ctx and rir_pctx
 */
#define rir_data_common(i_data)                 \
    ((struct rir_common*)i_data)

#define rir_data_rir(i_data)                    \
    ((struct rir_common*)i_data)->rir

#define rir_data_curr_fn(i_data)                    \
    ((struct rir_common*)i_data)->current_fn

#define rir_data_curr_block(i_data)                    \
    ((struct rir_common*)i_data)->current_block

#define rir_data_block_add(i_data, i_expr)                      \
    rir_common_block_add((struct rir_common*)i_data, i_expr)

#endif
