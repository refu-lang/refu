#include "llvm_conversion.h"
#include "llvm_values.h"
#include "llvm_types.h"
#include "llvm_ast.h"

#include <llvm-c/Core.h>

#include <types/type_elementary.h>
#include <ir/rir_expression.h>

static struct LLVMOpaqueValue *bllvm_compile_array_conversion(
    const struct rir_type *fromtype,
    const struct rir_type *totype,
    struct LLVMOpaqueValue *llvm_conv_val,
    struct LLVMOpaqueType *llvm_totype,
    struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef llvm_ret_val = NULL;
    const struct rir_type *array_member_type = fromtype->array.type;
    switch (fromtype->array.type->category) {
    case RIR_TYPE_ELEMENTARY:
        if (elementary_type_is_float(array_member_type->etype)) {
            llvm_ret_val =  LLVMBuildFPCast(ctx->builder, llvm_conv_val, llvm_totype, "");
        } else if (elementary_type_is_int(array_member_type->etype)) {
            size_t fromsize = rir_type_bytesize(array_member_type);
            size_t tosize = rir_type_bytesize(totype->array.type);
            if (fromsize < tosize) {
                llvm_ret_val = LLVMBuildZExt(ctx->builder, llvm_conv_val, llvm_totype, "");
            } else { //greater or equal size
                llvm_ret_val = LLVMBuildTruncOrBitCast(ctx->builder, llvm_conv_val, llvm_totype, "");
            }
        } else {
            RF_CRITICAL_FAIL("Unknown type of conversion");
        }
        break;
    case RIR_TYPE_COMPOSITE:
        // TODO: Should be a simple bitcast?
        RF_CRITICAL_FAIL("Array type conversion of composites not yet implemented");
        break;
    case RIR_TYPE_ARRAY:
        RF_CRITICAL_FAIL("Nested array conversion not implemented");
        break;
    }
    return llvm_ret_val;
}

struct LLVMOpaqueValue *bllvm_compile_conversion(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    struct rir_type *fromtype = expr->convert.val->type;
    const struct rir_type *totype = expr->convert.type;
    LLVMValueRef llvm_conv_val = bllvm_value_from_rir_value_or_die(expr->convert.val, ctx);
    LLVMTypeRef llvm_totype = bllvm_type_from_rir_type(totype, ctx);
    LLVMValueRef llvm_ret_val = NULL;

    switch (fromtype->category) {
    case RIR_TYPE_ELEMENTARY:
        if (elementary_type_is_float(fromtype->etype)) {
            llvm_ret_val =  LLVMBuildFPCast(ctx->builder, llvm_conv_val, llvm_totype, "");
        } else if (elementary_type_is_int(fromtype->etype)) {
            size_t fromsize = rir_type_bytesize(fromtype);
            size_t tosize = rir_type_bytesize(totype);
            if (fromsize < tosize) {
                llvm_ret_val = LLVMBuildZExt(ctx->builder, llvm_conv_val, llvm_totype, "");
            } else { //greater or equal size
                llvm_ret_val = LLVMBuildTruncOrBitCast(ctx->builder, llvm_conv_val, llvm_totype, "");
            }
        } else {
            RF_CRITICAL_FAIL("Unknown type of conversion");
        }
        break;
    case RIR_TYPE_COMPOSITE:
        RF_CRITICAL_FAIL("Unknown type of conversion");
        break;
    case RIR_TYPE_ARRAY:
        llvm_ret_val = bllvm_compile_array_conversion(
            fromtype,
            totype,
            llvm_conv_val,
            llvm_totype,
            ctx
        );
        break;
    }
    return llvm_ret_val;
}
