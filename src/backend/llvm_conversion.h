#ifndef LFR_BACKEND_LLVM_CONVERSION_H
#define LFR_BACKEND_LLVM_CONVERSION_H

struct LLVMOpaqueValue;
struct rir_expression;
struct llvm_traversal_ctx;

struct LLVMOpaqueValue *bllvm_compile_conversion(const struct rir_expression *expr,
                                                 struct llvm_traversal_ctx *ctx);

#endif
