#include "llvm_arrays.h"

#include <llvm-c/Core.h>

#include <ir/rir.h>
#include <ir/rir_expression.h>

#include "llvm_ast.h"
#include "llvm_utils.h"
#include "llvm_values.h"

LLVMValueRef bllvm_compile_fixedarr(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(expr->type == RIR_EXPRESSION_FIXEDARR, "unexpexted expression");
    const struct rir_fixedarr *fixedarr = &expr->fixedarr;
    LLVMTypeRef member_type = bllvm_type_from_rir_type(fixedarr->member_type, ctx);
    // can be NULL if there are no args
    LLVMValueRef *vals = bllvm_value_arr_to_values(&fixedarr->members, ctx);
    LLVMValueRef llvm_constarr = LLVMConstArray(
        member_type,
        vals,
        fixedarr->size
    );
    return llvm_constarr;
}

LLVMValueRef bllvm_compile_fixedarrsize(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(expr->type == RIR_EXPRESSION_FIXEDARRSIZE, "unexpexted expression");
    return bllvm_value_from_rir_value_or_die(&expr->val, ctx);
}

struct LLVMOpaqueValue *bllvm_compile_objidx(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(
        rir_type_is_array(expr->objidx.objmemory->type),
        "You can only get an index to an array type"
    );
    LLVMValueRef llvm_idxval = bllvm_value_from_rir_value_or_die(
        expr->objidx.idx,
        ctx
    );
    LLVMValueRef indices[] = {
        LLVMConstInt(LLVMInt32TypeInContext(ctx->llvm_context), 0, 0),
        llvm_idxval
    };
    LLVMValueRef gep = LLVMBuildGEP(
        ctx->builder,
        bllvm_value_from_rir_value_or_die(expr->objidx.objmemory, ctx),
        indices,
        2,
        ""
    );
    return LLVMBuildLoad(ctx->builder, gep, "");
}
