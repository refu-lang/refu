#ifndef LFR_IR_RIR_CONSTANT_H
#define LFR_IR_RIR_CONSTANT_H

#include <stdint.h>
#include <stdbool.h>

struct ast_node;
struct rir_expression;
struct rir_value;
struct rir_ctx;
struct rirtostr_ctx;

struct rir_object *rir_constant_create_obj(const struct ast_node *c, struct rir_ctx *ctx);
struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx);
struct rir_value *rir_constantval_fromint(int64_t n);

bool rir_constant_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);

#endif
