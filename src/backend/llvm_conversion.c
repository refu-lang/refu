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

#if 0
LLVMValueRef bllvm_compile_explicit_cast(const struct type *cast_type,
                                         struct ast_node *args,
                                         struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef cast_value = bllvm_compile_expression(
        args,
        ctx,
        RFLLVM_OPTION_IDENTIFIER_VALUE);
    // at the moment only cast to string requires special work
    if (cast_type->elementary.etype != ELEMENTARY_TYPE_STRING) {
        return bllvm_cast_value_to_elementary_maybe(cast_value, cast_type, ctx);
    }

    // from here and down it's a cast to string
    if (args->type == AST_CONSTANT) {
        RFS_PUSH();
        LLVMValueRef ret_str;
        struct RFstring *temps;
        switch (ast_constant_get_type(args)) {
        case CONSTANT_NUMBER_INTEGER:
            temps = RFS("%"PRIu64, args->constant.value.integer);
            break;
        case CONSTANT_NUMBER_FLOAT:
            // for now float conversion to string will use 4 decimal digits precision
            temps = RFS("%.4f", args->constant.value.floating);
            break;
        case CONSTANT_BOOLEAN:
            return args->constant.value.boolean
                ? bllvm_get_boolean_str(true, ctx)
                : bllvm_get_boolean_str(false, ctx);
        default:
            RF_CRITICAL_FAIL("Illegal constant number type encountered at code generation");
            break;
        }

        ret_str = bllvm_create_global_const_string(temps, ctx);
        RFS_POP();
        return ret_str;
    }

    if (type_is_specific_elementary(ast_node_get_type(args, AST_TYPERETR_DEFAULT), ELEMENTARY_TYPE_BOOL)) {
        LLVMBasicBlockRef taken_branch = bllvm_add_block_before_funcend(ctx);
        LLVMBasicBlockRef fallthrough_branch = bllvm_add_block_before_funcend(ctx);
        LLVMBasicBlockRef if_end = bllvm_add_block_before_funcend(ctx);
        LLVMValueRef string_alloca = LLVMBuildAlloca(ctx->builder, LLVMGetTypeByName(ctx->llvm_mod, "string"), "");
        LLVMBuildCondBr(ctx->builder, cast_value, taken_branch, fallthrough_branch);

        // if true
        bllvm_enter_block(ctx, taken_branch);
        bllvm_copy_string(bllvm_get_boolean_str(true, ctx),
                          string_alloca,
                          ctx);
        LLVMBuildBr(ctx->builder, if_end);
        // else false
        bllvm_enter_block(ctx, fallthrough_branch);
        bllvm_copy_string(bllvm_get_boolean_str(false, ctx),
                          string_alloca,
                          ctx);
        LLVMBuildBr(ctx->builder, if_end);
        bllvm_enter_block(ctx, if_end);
        return string_alloca;
    }

    // else
    RF_CRITICAL_FAIL("IIllegal cast, should not get here");
    return NULL;
}
#endif
