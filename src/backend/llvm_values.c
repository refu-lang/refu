#include "llvm_values.h"
#include "llvm_ast.h"
#include <Utils/sanity.h>

#include <llvm-c/Core.h>
#include <ir/rir_value.h>

void *bllvm_value_from_rir_value(const struct rir_value *v, struct llvm_traversal_ctx *ctx)
{
    return strmap_get(&ctx->valmap, &v->id);
}

void *bllvm_value_from_rir_value_or_die(const struct rir_value *v, struct llvm_traversal_ctx *ctx)
{
    struct LLVMOpaqueValue *ret = bllvm_value_from_rir_value(v, ctx);
    RF_ASSERT_OR_CRITICAL(ret, return NULL, "Mapping from rir value to llvm value was not found");
    return ret;
}

struct LLVMOpaqueValue **bllvm_value_arr_to_values(const struct value_arr *arr,
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
