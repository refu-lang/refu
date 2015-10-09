#include "llvm_conversion.h"
#include "llvm_values.h"
#include "llvm_types.h"
#include "llvm_ast.h"

#include <llvm-c/Core.h>

#include <types/type_elementary.h>
#include <ir/rir_expression.h>


struct LLVMOpaqueValue *bllvm_compile_conversion(const struct rir_expression *expr,
                                                 struct llvm_traversal_ctx *ctx)
{
    struct rir_ltype *fromtype = expr->convert.val->type;
    const struct rir_ltype *totype = expr->convert.type;
    LLVMValueRef llvm_conv_val = bllvm_value_from_rir_value_or_die(expr->convert.val, ctx);
    LLVMTypeRef llvm_totype = bllvm_type_from_rir_ltype(totype, ctx);
    LLVMValueRef llvm_ret_val = NULL;
    if (rir_ltype_is_elementary(fromtype)) {
        if (elementary_type_is_float(fromtype->etype)) {
            llvm_ret_val =  LLVMBuildFPCast(ctx->builder, llvm_conv_val, llvm_totype, "");
        } else if (elementary_type_is_int(fromtype->etype)) {
            size_t fromsize = rir_ltype_bytesize(fromtype);
            size_t tosize = rir_ltype_bytesize(totype);
            if (fromsize < tosize) {
                llvm_ret_val = LLVMBuildZExt(ctx->builder, llvm_conv_val, llvm_totype, "");
            } else { //greater or equal size
                llvm_ret_val = LLVMBuildTruncOrBitCast(ctx->builder, llvm_conv_val, llvm_totype, "");
            }
        } else {
            RF_CRITICAL_FAIL("Unknown type of conversion");
        }
    } else {
        RF_CRITICAL_FAIL("Unknown type of conversion");
    }
    return llvm_ret_val;
}
