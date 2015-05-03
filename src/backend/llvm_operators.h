#ifndef LFR_BACKEND_LLVM_OPERATORS_H
#define LFR_BACKEND_LLVM_OPERATORS_H

#include <stdbool.h>
#include <stdint.h>

struct llvm_traversal_ctx;
struct ast_node;
struct type;
struct LLVMOpaqueValue;

enum llvm_assign_options {
    BLLVM_ASSIGN_SIMPLE = 0x0,
    BLLVM_ASSIGN_MATCH_CASE = 0x1,
};

struct LLVMOpaqueValue *bllvm_compile_assign_llvm(
    struct LLVMOpaqueValue *from,
    struct LLVMOpaqueValue *to,
    const struct type *type,
    enum llvm_assign_options options,
    struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *bllvm_compile_assign(struct ast_node *from,
                                             struct ast_node *to,
                                             const struct type *common_type,
                                             struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *bllvm_compile_bop(struct ast_node *n,
                                          struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *bllvm_compile_uop(struct ast_node *n,
                                          struct llvm_traversal_ctx *ctx);
#endif
