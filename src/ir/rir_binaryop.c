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
    [BINARYOP_ASSIGN]            =   RIR_EXPRESSION_WRITE,
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

struct rir_expression *rir_binaryop_create_nonast(enum rir_expression_type type,
                                                  const struct rir_value *a,
                                                  const struct rir_value *b,
                                                  struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, type, ctx)) {
        free(ret);
        ret = NULL;
    } else {
        rir_binaryop_init(&ret->binaryop, a, b);
    }
    return ret;
}

struct rir_expression *rir_binaryop_create(const struct ast_binaryop *op,
                                           const struct rir_value *a,
                                           const struct rir_value *b,
                                           struct rir_ctx *ctx)
{
    return rir_binaryop_create_nonast(rir_binaryop_type_from_ast(op), a, b, ctx);
}

bool rir_process_binaryop(const struct ast_binaryop *op,
                          struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(op->left, ctx)) {
        goto fail;
    }
    struct rir_expression *lexpr = ctx->returned_expr;
    if (!rir_process_ast_node(op->right, ctx)) {
        goto fail;
    }
    struct rir_expression *rexpr = ctx->returned_expr;
    struct rir_expression *e = rir_binaryop_create(op, &lexpr->val, &rexpr->val, ctx);
    if (!e) {
        goto fail;
    }
    rirctx_block_add(ctx, e);
    RIRCTX_RETURN_EXPR(ctx, true, e);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

static const struct RFstring rir_bop_type_strings[] = {
    [RIR_EXPRESSION_ADD] = RF_STRING_STATIC_INIT("add"),
    [RIR_EXPRESSION_SUB] = RF_STRING_STATIC_INIT("sub"),
    [RIR_EXPRESSION_MUL] = RF_STRING_STATIC_INIT("mul"),
    [RIR_EXPRESSION_DIV] = RF_STRING_STATIC_INIT("div"),
    [RIR_EXPRESSION_CMP] = RF_STRING_STATIC_INIT("cmp"),
    [RIR_EXPRESSION_WRITE] = RF_STRING_STATIC_INIT("write"),
};

bool rir_binaryop_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    if (e->val.type == RIR_VALUE_NIL) {
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RITOSTR_INDENT RF_STR_PF_FMT"(" RF_STR_PF_FMT ", " RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(&rir_bop_type_strings[e->type]),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.a)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.b)))
            )) {
            return false;
        }
    } else {
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RITOSTR_INDENT RF_STR_PF_FMT" = "RF_STR_PF_FMT"(" RF_STR_PF_FMT ", " RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(&rir_bop_type_strings[e->type]),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.a)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.b)))
            )) {
            return false;
        }
    }

    return true;
}
