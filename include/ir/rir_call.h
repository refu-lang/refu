#ifndef LFR_IR_CALL_H
#define LFR_IR_CALL_H

#include <stdbool.h>
#include <stdint.h>
struct ast_node;
struct rir_value;
struct rir_call;
struct rir_expression;
struct rir_ctx;
struct rirtostr_ctx;

struct rir_object *rir_call_create_obj_from_ast(const struct ast_node *n, struct rir_ctx *ctx);
bool rir_call_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *call);
bool rir_process_fncall(const struct ast_node *n, struct rir_ctx *ctx);
/**
 * Get return type from a rir call
 *
 * @param c        The rir call whose return type to get
 * @param ctx      The rir context
 * @return         The return type of @a c
 */
struct rir_ltype *rir_call_return_type(struct rir_call *c, struct rir_ctx *ctx);

#endif
