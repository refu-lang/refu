#include <ir/rir_unaryop.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir_process.h>
#include <ir/rir_expression.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_utils.h>
#include <Utils/sanity.h>

bool rir_process_unaryop(const struct ast_unaryop *op, struct rir_ctx *ctx)
{
    struct rir_object *exprobj;
    const struct rir_value *opval = rir_process_ast_node_getreadval(op->operand, ctx);
    if (!opval) {
        RF_ERROR("A value should have been created for a unary operation operand");
        goto fail;
    }
    switch (op->type) {
    case UNARYOP_AMPERSAND:
        RF_CRITICAL_FAIL("TODO -- not yet implemented");
        break;
    case UNARYOP_INC:
        if (!(exprobj = rir_binaryop_create_nonast_obj(RIR_EXPRESSION_ADD, opval, &g_rir_const_1, ctx))) {
            RF_ERROR("Failed to create rir expression out of increase unary operation");
            goto fail;
        }
        break;
    case UNARYOP_DEC:
        if (!(exprobj = rir_binaryop_create_nonast_obj(RIR_EXPRESSION_ADD, opval, &g_rir_const_m1, ctx))) {
            RF_ERROR("Failed to create rir expression out of decreate unary operation");
            goto fail;
        }
        break;
    case UNARYOP_MINUS:
        if (!(exprobj = rir_binaryop_create_nonast_obj(RIR_EXPRESSION_MUL, opval, &g_rir_const_m1, ctx))) {
            RF_ERROR("Failed to create rir expression out of minus unary operation");
            goto fail;
        }
        break;
    case UNARYOP_PLUS:
        RF_CRITICAL_FAIL("What was this unary op supposed to do?");
        break;
    }
    // finally add the generated expression to the block and succeed
    rirctx_block_add(ctx, &exprobj->expr);
    RIRCTX_RETURN_EXPR(ctx, true, exprobj);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}
