#ifndef LFR_IR_UTILS_H
#define LFR_IR_UTILS_H

#include <stdbool.h>
#include <ir/rir_value.h>

enum rir_code_loc {
    RIR_LOC_FIXEDARR,
    RIR_LOC_CONVERSION,
    RIR_LOC_FNDECL_ARGUMENTS,
    RIR_LOC_FNDECL_RETURN,
    RIR_LOC_TYPEDESC,
    RIR_LOC_FNCALL_MAYBE_CONVERT,
    RIR_LOC_FNCALL_SUM_CALL,
    RIR_LOC_SYMBOL_TABLE_ALLOCA,
};

extern struct rir_value g_rir_const_1;
extern struct rir_value g_rir_const_m1;
extern struct rir_type g_rir_i32_type;

/**
 * Initialize all the utils needed by all rir functions
 */
bool rir_utils_create();
void rir_utils_destroy();

/**
 * Get the given value or perform a read if it's a pointer
 */
struct rir_value *rirctx_getread_val(struct rir_value *v, struct rir_ctx *ctx);
/**
 * Get the value of the given expression. Performs a read if necessary
 */
struct rir_value *rirctx_getread_exprval(struct rir_expression *e, struct rir_ctx *ctx);
/**
 * Acts like @ref rir_getread_val() but returns the expression itself and not the value
 */
struct rir_expression *rirctx_getread_expr(struct rir_expression *e, struct rir_ctx *ctx);
/**
 * Acts like @ref rir_getread_val() but returns the rir object containing the expression
 */
struct rir_object *rirctx_getread_obj(struct rir_object *e, struct rir_ctx *ctx);


/**
 * Allocates a rir object, writes to it and adds both operations to
 * the current block
 *
 * @param t                   The rir type of which to allocate an object
 * @param value_to_write      The rir value to write to the allocated object
 * @param ctx                 The rir context
 * @return                    The rir_object that was created for the allocation
 *                            or NULL in failure.
 */
struct rir_object *rirctx_alloc_write_add(
    struct rir_type *t,
    struct rir_value *value_to_write,
    struct rir_ctx *ctx
);

#endif
