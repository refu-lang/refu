#include <ir/rir_process.h>
#include <ir/rir.h>
#include <ir/rir_binaryop.h>
#include <ir/rir_unaryop.h>
#include <ir/rir_block.h>
#include <ir/rir_constant.h>
#include <ir/rir_function.h>
#include <ir/rir_call.h>
#include <ast/returnstmt.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>

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
    if (!rir_ctx_curr_fn(ctx)->retslot_val) {
        RF_ERROR("Could not find the return alloca of a function");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    struct rir_expression *e = rir_write_create(
        rir_ctx_curr_fn(ctx)->retslot_val,
        ret_val,
        RIRPOS_AST,
        ctx
    );
    if (!e) {
        RF_ERROR("Failed to create a write() during rir return processing");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    rir_common_block_add(&ctx->common, e);
    // jump to the return
    if (!rir_block_exit_init_branch(&rir_ctx_curr_block(ctx)->exit, rir_ctx_curr_fn(ctx)->end_label)) {
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

static bool rir_process_strlit(const struct ast_node *n,
                               struct rir_ctx *ctx)
{
    struct rir_object *litobj = rir_strlit_obj(rir_ctx_rir(ctx), n);
    if (!litobj) {
        RF_ERROR("A string literal was not found in the global string literals");
        RIRCTX_RETURN_EXPR(ctx, false, NULL);
    }
    RIRCTX_RETURN_EXPR(ctx, true, litobj);
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
    case AST_UNARY_OPERATOR:
        return rir_process_unaryop(&n->unaryop, ctx);
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
    case AST_STRING_LITERAL:
        return rir_process_strlit(n, ctx);
    case AST_CONDITIONAL_BRANCH:
    case AST_MATCH_CASE:
        // Do nothing in these cases
        return true;
    default:
        RF_CRITICAL_FAIL("Not yet implemented expression for RIR");
    }
    return false;
}

i_INLINE_INS struct rir_object *rir_process_ast_node_getobj(const struct ast_node *n,
                                                             struct rir_ctx *ctx);
i_INLINE_INS struct rir_value *rir_process_ast_node_getval(const struct ast_node *n,
                                                           struct rir_ctx *ctx);
i_INLINE_INS const struct rir_value *rir_process_ast_node_getreadval(const struct ast_node *n,
                                                                     struct rir_ctx *ctx);
