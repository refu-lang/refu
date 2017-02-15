#include "llvm_operators.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>

#include <rfbase/utils/bits.h>

#include <ast/ast.h>
#include <ast/operators.h>
#include <types/type_elementary.h>
#include <types/type_comparisons.h>
#include <types/type.h>
#include <ir/rir_expression.h>
#include <ir/rir_type.h>

#include "llvm_ast.h"
#include "llvm_utils.h"
#include "llvm_values.h"

LLVMValueRef bllvm_compile_comparison(const struct rir_expression *expr,
                                      struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef left = bllvm_value_from_rir_value_or_die(expr->binaryop.a, ctx);
    LLVMValueRef right = bllvm_value_from_rir_value_or_die(expr->binaryop.b, ctx);
    struct rir_type *typea = expr->binaryop.a->type;
    struct rir_type *typeb = expr->binaryop.b->type;
    RF_ASSERT(rir_type_is_elementary(typea), "Backend comparisons should only happen with elementary types");
    RF_ASSERT(rir_type_is_elementary(typeb), "Backend comparisons should only happen with elementary types");
    RF_ASSERT(typea->etype == typeb->etype, "Comparison should only happen with same type");

    // TODO: Maybe take into account signedness?
    if (elementary_type_is_float(typea->etype)) {
        LLVMRealPredicate llvm_real_compare_type;
        switch(expr->type) {
        case RIR_EXPRESSION_CMP_EQ:
                llvm_real_compare_type = LLVMRealOEQ;
            break;
        case RIR_EXPRESSION_CMP_NE:
                llvm_real_compare_type = LLVMRealONE;
            break;
        case RIR_EXPRESSION_CMP_GE:
                llvm_real_compare_type = LLVMRealOGE;
            break;
        case RIR_EXPRESSION_CMP_GT:
                llvm_real_compare_type = LLVMRealOGT;
            break;
        case RIR_EXPRESSION_CMP_LE:
                llvm_real_compare_type = LLVMRealOLE;
            break;
        case RIR_EXPRESSION_CMP_LT:
                llvm_real_compare_type = LLVMRealOLT;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at comparison code generation");
            return NULL;
            break;
        }
        return LLVMBuildFCmp(ctx->builder, llvm_real_compare_type, left, right, "");
    }
    LLVMIntPredicate llvm_int_compare_type;
    switch(expr->type) {
    case RIR_EXPRESSION_CMP_EQ:
        llvm_int_compare_type = LLVMIntEQ;
        break;
    case RIR_EXPRESSION_CMP_NE:
        llvm_int_compare_type = LLVMIntNE;
        break;
        /*
         * TODO: All 4 of GE/GT/LE/LT used to be UGE/UGT/ULE/ULT
         *       Naturally this does not really work when we compare
         *       signed numbers. Move the abstraction to the RIR so that
         *       we can define the type of the comparison there since at the
         *       moment we get here in the LLVM backend it's hard to see whether
         *       we are comparing signed or unsigned integers.
         *
         */
    case RIR_EXPRESSION_CMP_GE:
        llvm_int_compare_type = LLVMIntSGE;
        break;
    case RIR_EXPRESSION_CMP_GT:
        llvm_int_compare_type = LLVMIntSGT;
        break;
    case RIR_EXPRESSION_CMP_LE:
        llvm_int_compare_type = LLVMIntSLE;
        break;
    case RIR_EXPRESSION_CMP_LT:
        llvm_int_compare_type = LLVMIntSLT;
        break;
    default:
            RF_CRITICAL_FAIL("Illegal operand types at comparison code generation");
            return NULL;
        break;
    }
    return LLVMBuildICmp(ctx->builder, llvm_int_compare_type, left, right, "");
}

struct LLVMOpaqueValue *bllvm_compile_rirbop(const struct rir_expression *expr,
                                             struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef ret;
    LLVMValueRef left = bllvm_value_from_rir_value_or_die(expr->binaryop.a, ctx);
    LLVMValueRef right = bllvm_value_from_rir_value_or_die(expr->binaryop.b, ctx);
    switch(expr->type) {
    case RIR_EXPRESSION_ADD:
        ret = LLVMBuildAdd(ctx->builder, left, right, "");
        break;
    case RIR_EXPRESSION_SUB:
        ret = LLVMBuildSub(ctx->builder, left, right, "");
        break;
    case RIR_EXPRESSION_MUL:
        ret = LLVMBuildMul(ctx->builder, left, right, "");
        break;
    case RIR_EXPRESSION_DIV:
        ret = LLVMBuildUDiv(ctx->builder, left, right, "");
        break;
    default:
        RF_CRITICAL_FAIL("Should never get anything other than binaryop here");
        break;
    }
    return ret;
}
