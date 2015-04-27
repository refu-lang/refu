#ifndef LFR_BACKEND_LLVM_MATCHEXPR_H
#define LFR_BACKEND_LLVM_MATCHEXPR_H

struct LLVMOpaqueValue;

struct ast_node;
struct llvm_traversal_ctx;

struct LLVMOpaqueValue *bllvm_compile_matchexpr(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx);

#endif
