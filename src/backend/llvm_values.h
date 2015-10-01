#ifndef LFR_BACKEND_LLVM_VALUES_H
#define LFR_BACKEND_LLVM_VALUES_H

struct rir_value;
struct LLVMOpaqueValue;
struct llvm_traversal_ctx;
struct value_arr;

void *bllvm_value_from_rir_value(const struct rir_value *v, struct llvm_traversal_ctx *ctx);
void *bllvm_value_from_rir_value_or_die(const struct rir_value *v, struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue **bllvm_value_arr_to_values(const struct value_arr *arr,
                                                   struct llvm_traversal_ctx *ctx);

#endif
