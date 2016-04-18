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
    // can be NULL if there are no args
    LLVMValueRef *vals = bllvm_value_arr_to_values(&fixedarr->members, ctx);
    LLVMValueRef llvm_constarr = LLVMConstArray(
        bllvm_type_from_rir_type(expr->val.type, ctx),
        vals,
        fixedarr->size
    );
    return llvm_constarr;
}

LLVMValueRef bllvm_compile_constfixedarr(
    const struct rir_fixedarr *fixedarr,
    struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef ret;
    // can be NULL if there are no args
    LLVMValueRef *vals = bllvm_value_arr_to_values(&fixedarr->members, ctx);
    ret = LLVMConstArray(
        bllvm_type_from_rir_type(fixedarr->member_type, ctx),
        vals,
        fixedarr->size
    );
    return ret;
}
