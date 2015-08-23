#include <ir/rir_binaryop.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <ir/rir_block.h>
#include <ast/operators.h>


static const enum rir_expression_type binaryop_operation_to_rir[] = {
    [BINARYOP_ADD]               =   RIR_EXPRESSION_ADD,
    [BINARYOP_SUB]               =   RIR_EXPRESSION_SUB,
    [BINARYOP_MUL]               =   RIR_EXPRESSION_MUL,
    [BINARYOP_DIV]               =   RIR_EXPRESSION_DIV,

    [BINARYOP_CMP_EQ]            =   RIR_EXPRESSION_CMP,
    [BINARYOP_CMP_NEQ]           =   RIR_EXPRESSION_CMP,
    [BINARYOP_CMP_GT]            =   RIR_EXPRESSION_CMP,
    [BINARYOP_CMP_GTEQ]          =   RIR_EXPRESSION_CMP,
    [BINARYOP_CMP_LT]            =   RIR_EXPRESSION_CMP,
    [BINARYOP_CMP_LTEQ]          =   RIR_EXPRESSION_CMP,

    [BINARYOP_LOGIC_AND]         =   RIR_EXPRESSION_LOGIC_AND,
    [BINARYOP_LOGIC_OR]          =   RIR_EXPRESSION_LOGIC_OR,
};

enum binaryop_type rir_binaryop_type_from_ast(const struct ast_binaryop *op)
{
    return binaryop_operation_to_rir[op->type];
}

static inline void rir_binaryop_init(struct rir_binaryop *rbop,
                                     const struct rir_value *a,
                                     const struct rir_value *b)
{
    rbop->a = a;
    rbop->b = b;
}

struct rir_expression *rir_binaryop_create(const struct ast_binaryop *op,
                                           const struct rir_value *a,
                                           const struct rir_value *b,
                                           struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, rir_binaryop_type_from_ast(op), ctx)) {
        free(ret);
        ret = NULL;
    } else {
        rir_binaryop_init(&ret->binaryop, a, b);
    }
    return ret;
}

struct rir_expression *rir_process_binaryop(const struct ast_binaryop *op,
                                            struct rir_ctx *ctx)
{
    struct rir_expression *lexpr = rir_process_ast_node(op->left, ctx);
    struct rir_expression *rexpr = rir_process_ast_node(op->right, ctx);
    struct rir_expression *e = rir_binaryop_create(op, &lexpr->val, &rexpr->val, ctx);
    rirctx_block_add(ctx, e);
    return e;
}
