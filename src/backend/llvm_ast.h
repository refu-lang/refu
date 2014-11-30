#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H
#include <stdbool.h>

struct ast_node;
struct compiler_args;

struct LLVMOpaqueModule;
struct LLVMOpaqueBuilder;

struct llvm_traversal_ctx {
    struct LLVMOpaqueModule *mod;
    struct LLVMOpaqueBuilder *builder;
    struct compiler_args *args;
};
bool backend_llvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root);
#endif
