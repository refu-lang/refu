#ifndef LFR_IR_RIR_CONSTANT_H
#define LFR_IR_RIR_CONSTANT_H

#include <stdint.h>
#include <stdbool.h>

struct ast_node;
struct rir_expression;
struct rir_value;
struct rir_ctx;
struct rir;
struct rirtostr_ctx;

struct rir_object *rir_constant_create_obj(const struct ast_node *c, struct rir_ctx *ctx);
struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx);
/**
 * Create a rir constant value from an int64
 *
 * Creates a value that is not meant to belong to any specific rir object. As
 * such the value will also be stored under the given rir module so that it can
 * later be freed.
 *
 * @param n        The integer to create the constant from
 * @param r        The rir module object under which the value will be stored.
 * @return         A pointer to the allocated value
 */
struct rir_value *rir_constantval_create_fromint64(int64_t n, struct rir* r);
bool rir_constantval_init_fromint64(struct rir_value *v, int64_t n);

struct rir_value *rir_constantval_create_fromint32(int32_t n, struct rir* r);
bool rir_constantval_init_fromint32(struct rir_value *v, int32_t n);

const struct RFstring *rir_constant_string(const struct rir_value *val);
bool rir_constant_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);

#endif
