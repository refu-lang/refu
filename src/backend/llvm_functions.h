#ifndef LFR_BACKEND_LLVM_FUNCTIONS_H
#define LFR_BACKEND_LLVM_FUNCTIONS_H

#include <stdbool.h>

struct LLVMOpaqueModule;
struct LLVMOpaqueBasicBlock;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

struct rir;
struct ast_node;
struct rir_function;
struct rir_call;
struct llvm_traversal_ctx;

struct LLVMOpaqueValue *bllvm_compile_functioncall(const struct rir_call *call,
                                                   struct llvm_traversal_ctx *ctx);

/**
 *  Returns the LLVMTypeRef of an LLVMValueRef that's a function
 */
struct LLVMOpaqueType *bllvm_function_type(struct LLVMOpaqueValue *fn);

bool bllvm_create_module_functions(struct rir *r, struct llvm_traversal_ctx *ctx);
#endif
