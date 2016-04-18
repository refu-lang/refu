#include "llvm_values.h"

#include <llvm-c/Core.h>

#include <rflib/utils/sanity.h>

#include <ir/rir_value.h>
#include <ir/rir_function.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

void *bllvm_value_from_rir_value(
    const struct rir_value *v,
    struct llvm_traversal_ctx *ctx)
{
    // constant values are easy
    if (v->category == RIR_VALUE_CONSTANT) {
        return bllvm_compile_constant(&v->constant, v->type, ctx);
    }
    // so are string literals
    if (v->category == RIR_VALUE_LITERAL) {
        return bllvm_compile_literal(&v->literal, ctx);
    }
    // otherwise search the mapping
    void *ret =  strmap_get(&ctx->valmap, &v->id);
    if (!ret) {
        // if not found in rir val to llvm map, it may not have been added yet.
        // This can happen for function arguments so check if value is one
        int arg_idx;
        if ((arg_idx = rir_fndef_value_to_argnum(ctx->current_rfn, v)) != -1) {
            ret = LLVMGetParam(ctx->current_function, arg_idx);
            // also add it to the rir val to llvm map
            if (!llvm_traversal_ctx_map_llvmval(ctx, v, ret)) {
                RF_ERROR("Failed to add a rir argument to rir value to llvm mapping");
                ret = NULL;
            }
        }// else it's a failure, with ret == NULL
    }
    return ret;
}

void *bllvm_value_from_rir_value_or_die(
    const struct rir_value *v,
    struct llvm_traversal_ctx *ctx)
{
    struct LLVMOpaqueValue *ret = bllvm_value_from_rir_value(v, ctx);
    RF_ASSERT_OR_CRITICAL(ret, return NULL, "Mapping from rir value to llvm value was not found");
    return ret;
}

struct LLVMOpaqueValue **bllvm_value_arr_to_values(
    const struct value_arr *arr,
    struct llvm_traversal_ctx *ctx)
{
    llvm_traversal_ctx_reset_values(ctx);
    struct rir_value **val;
    LLVMValueRef llvm_val;
    darray_foreach(val, *arr) {
        llvm_val = bllvm_value_from_rir_value_or_die(*val, ctx);
        llvm_traversal_ctx_add_value(ctx, llvm_val);
    }
    return llvm_traversal_ctx_get_values(ctx);
}
