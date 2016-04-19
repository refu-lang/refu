#ifndef LFR_BACKEND_LLVM_ARRAYS_H
#define LFR_BACKEND_LLVM_ARRAYS_H

struct LLVMOpaqueValue;
struct llvm_traversal_ctx;
struct rir_expression;
struct rir_fixedarr;
struct rir;

struct LLVMOpaqueValue *bllvm_compile_fixedarr(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx
);

struct LLVMOpaqueValue *bllvm_compile_objidx(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx
);
#endif
