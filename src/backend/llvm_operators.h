#ifndef LFR_BACKEND_LLVM_OPERATORS_H
#define LFR_BACKEND_LLVM_OPERATORS_H

#include <stdbool.h>
#include <stdint.h>

struct llvm_traversal_ctx;
struct ast_node;
struct type;
struct LLVMOpaqueValue;

struct LLVMOpaqueValue *bllvm_compile_assign_llvm(
    struct LLVMOpaqueValue *from,
    struct LLVMOpaqueValue *to,
    const struct type *type,
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
