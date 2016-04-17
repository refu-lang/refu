#ifndef LFR_IR_CALL_H
#define LFR_IR_CALL_H

#include <stdbool.h>
#include <stdint.h>
#include <ir/rir_common.h>

struct ast_node;
struct rir_value;
struct value_arr;
struct rir_call;
struct rir_expression;
struct rir_ctx;
struct rir_common;
struct rirtostr_ctx;
struct RFstring;

struct rir_object *rir_call_create_obj_from_ast(const struct ast_node *n, struct rir_ctx *ctx);
struct rir_object *rir_call_create_obj(
    const struct RFstring *name,
    struct value_arr *args,
    bool is_foreign,
    enum rir_pos pos,
    rir_data data
);
bool rir_call_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *call);
bool rir_process_fncall(const struct ast_node *n, struct rir_ctx *ctx);

struct rir_expression *rir_call_to_expr(const struct rir_call *c);
/**
 * Get return type from a rir call
 *
 * @param c        The rir call whose return type to get
 * @param cm       The rir common data
 * @return         The return type of @a c
 */
struct rir_type *rir_call_return_type(struct rir_call *c, struct rir_common *cm);

void rir_call_deinit(struct rir_call *c);
#endif
