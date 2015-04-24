#ifndef LFR_BACKEND_LLVM_UTILS_H
#define LFR_BACKEND_LLVM_UTILS_H

struct LLVMOpaqueModule;
struct LLVMOpaqueBasicBlock;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

struct rir_type;
struct llvm_traversal_ctx;

/* -- Some debugging functions to help in sticky situations -- */
void bllvm_val_debug(struct LLVMOpaqueValue *v, const char *val_name);
void bllvm_type_debug(struct LLVMOpaqueType *t,
                      const char *type_name,
                      struct llvm_traversal_ctx *ctx);
void bllvm_mod_debug(struct LLVMOpaqueModule *m, const char *mod_name);

void bllvm_assign_to_string(struct LLVMOpaqueValue *string_alloca,
                            struct LLVMOpaqueValue *length,
                            struct LLVMOpaqueValue *string_data,
                            struct llvm_traversal_ctx *ctx);
void bllvm_load_from_string(struct LLVMOpaqueValue *string_alloca,
                            struct LLVMOpaqueValue **length,
                            struct LLVMOpaqueValue **string_data,
                            struct llvm_traversal_ctx *ctx);
/**
 * A combination of a @ref bllvm_load_from_string() and a
 * @ref bllvm_assign_to_string() to achieve a shallow copy
 * of a string.
 */
void bllvm_copy_string(struct LLVMOpaqueValue *from,
                       struct LLVMOpaqueValue *to,
                       struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *bllvm_cast_value_to_type_maybe(
    struct LLVMOpaqueValue *val,
    struct LLVMOpaqueType *type,
    struct llvm_traversal_ctx *ctx);

void bllvm_store(struct LLVMOpaqueValue *val, struct LLVMOpaqueValue *ptr,
                 struct llvm_traversal_ctx *ctx);

/**
 * Inserts a new block to a function, right before the last (the returning)
 * block of said function.
 *
 * @param ctx             The llvm traversal context
 * @return                The inserted block
 */
struct LLVMOpaqueBasicBlock *bllvm_add_block_before_funcend(
    struct llvm_traversal_ctx *ctx);

/**
 * Positions the builder at end of @c block and makes it current
 *
 * @param ctx             The llvm traversal context
 * @param block           The block to enter
 */
void bllvm_enter_block(struct llvm_traversal_ctx *ctx,
                       struct LLVMOpaqueBasicBlock *block);

/**
 * Add branch to label unless last instruction was already a branch command
 *
 * @param    The target block to jump to 
 * @param    ctx the llvm context
 */
struct LLVMOpaqueValue *bllvm_add_br(struct LLVMOpaqueBasicBlock *target,
                                     struct llvm_traversal_ctx *ctx);

/**
 * Shallow copy/assignment of designed types
 * @param from      The source llvm value to assign to
 * @param to        The destination llvm value
 * @param ctx             The llvm traversal context
 */
void bllvm_assign_defined_types(struct LLVMOpaqueValue *from,
                                struct LLVMOpaqueValue *to,
                                struct llvm_traversal_ctx *ctx);
#endif
