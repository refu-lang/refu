#ifndef LFR_IR_RIR_CONVERT_H
#define LFR_IR_RIR_CONVERT_H

#include <stdbool.h>

struct rir_expression;
struct rir_value;
struct rir_type;
struct rir_ctx;
struct ast_node;

struct rir_object *rir_convert_create_obj(const struct rir_value *convval,
                                          const struct rir_type *totype,
                                          struct rir_ctx *ctx);

bool rir_process_convertcall(const struct ast_node *n, struct rir_ctx *ctx);

/**
 * Check if a value is of a specific type and if not do a conversion
 *
 * @param val            The value to check
 * @param checktype      The type to check for equality with val's type
 * @return               If the passed value's type is equal to @a checktype
 *                       then @a val itself is returned. If not a conversion
 *                       instruction is added and the conversion's value is returned
 *                       The new instruction gets a copy of @a checktype for its type.
 */
const struct rir_value *rir_maybe_convert(const struct rir_value *val,
                                          const struct rir_type *checktype,
                                          struct rir_ctx *ctx);

/**
 * Acts just like @ref rir_maybe_convert() except that @a checktype
 * is acquired by the function and should not be accessed after it.
 */
const struct rir_value *rir_maybe_convert_acquire_type(const struct rir_value *val,
                                                       struct rir_type *checktype,
                                                       struct rir_ctx *ctx);

/**
 * Creates a convert object and adds it to the current block if the returned
 * object was a rir expression.
 *
 * @param convval            The value for conversion
 * @param totype             The type to convert to
 * @return                   A rir object representing the conversion. It's
 *                           either a rir expression in which case it's a
 *                           convert() rir instruction or a rir global since
 *                           conversion was done at compile time. Null in error.
 */
struct rir_object *rir_convert_create_obj_maybeadd(const struct rir_value *convval,
                                                   const struct rir_type *totype,
                                                   struct rir_ctx *ctx);
#endif
