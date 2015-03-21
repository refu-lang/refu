#ifndef LFR_BACKEND_LLVM_OPERATORS_H
#define LFR_BACKEND_LLVM_OPERATORS_H

#include <stdbool.h>
#include <stdint.h>

struct llvm_traversal_ctx;
struct ast_node;
struct LLVMOpaqueValue;

struct LLVMOpaqueValue *backend_llvm_compile_bop(struct ast_node *n,
                                                 struct llvm_traversal_ctx *ctx);
struct LLVMOpaqueValue *backend_llvm_compile_uop(struct ast_node *n,
                                                 struct llvm_traversal_ctx *ctx);
#endif
