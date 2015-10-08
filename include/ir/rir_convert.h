#ifndef LFR_IR_RIR_CONVERT_H
#define LFR_IR_RIR_CONVERT_H

#include <stdbool.h>

struct rir_expression;
struct rir_value;
struct rir_ltype;
struct rir_ctx;

struct rir_object *rir_convert_create_obj(const struct rir_value *convval,
                                          const struct rir_ltype *totype,
                                          struct rir_ctx *ctx);
struct rir_expression *rir_convert_create(const struct rir_value *convval,
                                          const struct rir_ltype *totype,
                                          struct rir_ctx *ctx);

/**
 * Check if a vale is of a specific type and if not do a conversion
 *
 * @param val            The value to check
 * @param checktype      The type to check for equality with val's type
 * @return               If the passed value's type is equal to @a checktype
 *                       then @a val itself is returned. If not a conversion
 *                       instruction is added and the conversion's value is returned
 */
const struct rir_value *rir_maybe_convert(const struct rir_value *val,
                                          const struct rir_ltype *checktype,
                                          struct rir_ctx *ctx);
#endif
