#ifndef LFR_BACKEND_LLVM_FUNCTIONS_H
#define LFR_BACKEND_LLVM_FUNCTIONS_H

struct LLVMOpaqueModule;
struct LLVMOpaqueBasicBlock;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

struct ast_node;
struct rir_function;
struct llvm_traversal_ctx;

struct LLVMOpaqueValue *bllvm_compile_function(struct ast_node *fn,
                                               struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *bllvm_compile_functioncall(struct ast_node *n,
                                                   struct llvm_traversal_ctx *ctx);

/**
 *  Returns the LLVMTypeRef of an LLVMValueRef that's a function
 */
struct LLVMOpaqueType *bllvm_function_type(struct LLVMOpaqueValue *fn);
#endif
