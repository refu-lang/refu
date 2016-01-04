#ifndef LFR_IR_RIR_BINARYOP_H
#define LFR_IR_RIR_BINARYOP_H

#include <ir/rir_expression.h>


struct ast_binaryop;

struct rir_expression *rir_binaryop_create(
    const struct ast_binaryop *op,
    const struct rir_value *a,
    const struct rir_value *b,
    enum rir_pos pos,
    rir_data data
);

struct rir_object *rir_binaryop_create_nonast_obj(
    enum rir_expression_type type,
    const struct rir_value *a,
    const struct rir_value *b,
    enum rir_pos pos,
    rir_data data
);

struct rir_expression *rir_binaryop_create_nonast(
    enum rir_expression_type type,
    const struct rir_value *a,
    const struct rir_value *b,
    enum rir_pos pos,
    rir_data data
);

bool rir_process_binaryop(const struct ast_binaryop *n, struct rir_ctx *ctx);
bool rir_binaryop_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);
#endif
