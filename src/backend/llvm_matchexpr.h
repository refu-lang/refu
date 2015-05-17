#ifndef LFR_BACKEND_LLVM_MATCHEXPR_H
#define LFR_BACKEND_LLVM_MATCHEXPR_H

struct LLVMOpaqueValue;

struct ast_node;
struct llvm_traversal_ctx;

/**
 * Compile a matchexpression
 *
 * @param n               The match expression node to compile
 * @param ctx             The llvm traversal context
 * @return                Returns either an LLVMValue with the result of the
 *                        compiled match expression or NULL if the match expression
 *                        has void result
 */
struct LLVMOpaqueValue *bllvm_compile_matchexpr(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx);

#endif
