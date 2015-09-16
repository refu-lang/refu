#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_block.h>
#include <ir/rir_constant.h>
#include <ir/rir_function.h>
#include <ir/rir_call.h>
#include <ast/returnstmt.h>
#include <ast/type.h>
#include <ast/vardecl.h>

static bool rir_process_vardecl(const struct ast_node *n,
                                struct rir_ctx *ctx)
{
    // Just return the value from the symbol table
    struct ast_node *left = ast_types_left(ast_vardecl_desc_get(n));
    const struct RFstring *id = ast_identifier_str(left);
    struct rir_object *varobj = rir_ctx_st_getobj(ctx, id);
    if (!varobj) {
        RF_ERROR("Could not find a vardecl's RIR object in the symbol table");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, varobj);
}

static bool rir_process_return(const struct ast_node *n,
                               struct rir_ctx *ctx)
{
    if (!rir_process_ast_node(ast_returnstmt_expr_get(n), ctx)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_value *ret_val = rir_ctx_lastval_get(ctx);
    // write the return value to the return slot
    struct rir_expression *ret_slot = rir_fnmap_get_returnslot(ctx);
    if (!ret_slot) {
        RF_ERROR("Could not find the returnvalue of a function in the string map");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *e = rir_binaryop_create_nonast(RIR_EXPRESSION_WRITE, &ret_slot->val, ret_val, ctx);
    rirctx_block_add(ctx, e);
    // jump to the return
    if (!rir_block_exit_init_branch(&ctx->current_block->exit, ctx->current_fn->end_label)) {
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }

    RIRCTX_RETURN_EXPR(ctx, true, NULL);
}

static bool rir_process_constant(const struct ast_node *n,
                                 struct rir_ctx *ctx)
{
    struct rir_object *ret_expr = rir_constant_create_obj(n, ctx);
    RIRCTX_RETURN_EXPR(ctx, true, ret_expr);
}

bool rir_process_identifier(const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_ctx_st_getobj(ctx, ast_identifier_str(n));
    if (!obj) {
        RF_ERROR("An identifier was not found in the strmap during rir creation");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, obj);
}

bool rir_process_ast_node(const struct ast_node *n,
                          struct rir_ctx *ctx)
{
    switch (n->type) {
    case AST_IF_EXPRESSION:
        return rir_process_ifexpr(n, ctx);
    case AST_VARIABLE_DECLARATION:
        return rir_process_vardecl(n, ctx);
    case AST_BINARY_OPERATOR:
        return rir_process_binaryop(&n->binaryop, ctx);
    case AST_RETURN_STATEMENT:
        return rir_process_return(n, ctx);
    case AST_CONSTANT:
        return rir_process_constant(n, ctx);
    case AST_IDENTIFIER:
        return rir_process_identifier(n, ctx);
    case AST_FUNCTION_CALL:
        return rir_process_fncall(n, ctx);
    case AST_MATCH_EXPRESSION:
        return rir_process_matchexpr((struct ast_node*)n, ctx);
    case AST_CONDITIONAL_BRANCH:
    case AST_MATCH_CASE:
        // Do nothing in these cases
        return true;
    default:
        RF_CRITICAL_FAIL("Not yet implemented expression for RIR");
    }
    return false;
}
