#ifndef LFR_BACKEND_LLVM_AST_H
#define LFR_BACKEND_LLVM_AST_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

struct ast_node;
struct analyzer;
struct symbol_table;
struct compiler_args;

struct LLVMOpaqueModule;
struct LLVMOpaqueBuilder;

struct llvm_traversal_ctx {
    struct LLVMOpaqueModule *mod;
    struct LLVMOpaqueBuilder *builder;
    struct {darray(struct LLVMOpaqueType*);} params;

    struct analyzer *analyzer;
    struct compiler_args *args;
    struct symbol_table *current_st;
};
bool backend_llvm_create_ir_ast(struct llvm_traversal_ctx *ctx,
                                struct ast_node *root);
#endif
