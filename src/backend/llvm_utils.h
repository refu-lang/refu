#ifndef LFR_BACKEND_LLVM_UTILS_H
#define LFR_BACKEND_LLVM_UTILS_H

#include <stdint.h>

struct LLVMOpaqueModule;
struct LLVMOpaqueBasicBlock;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;
struct LLVMOpaqueTargetData;

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
 * Prints the LLVM error string and disposes of it
 */
void bllvm_error(const char *errpre, char **llvmerr);
void bllvm_error_dispose(char **llvmerr);


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
 * Create a new fatal failure block another block
 *
 * @param target        The block before which to add the fail block
 * @param exit_code     The code to pass to the exit() function
 * @param ctx           The llvm traversal context
 */
struct LLVMOpaqueBasicBlock *bllvm_add_fatal_block_before(struct LLVMOpaqueBasicBlock *target,
                                                          int exit_code,
                                                          struct llvm_traversal_ctx *ctx);

/**
 * Add branch to label unless last instruction was already a branch command
 *
 * @param    The target block to jump to 
 * @param    ctx the llvm context
 */
struct LLVMOpaqueValue *bllvm_add_br(struct LLVMOpaqueBasicBlock *target,
                                     struct llvm_traversal_ctx *ctx);

/**
 * Memcpy a pointer value.
 *
 * Size is determined by the function as the size of the pointed to element of
 * the @a from value.
 * If size of @a to is less than size of @a from then this function will fail.
 * 
 * @param from      The source llvm value to assign to
 * @param to        The destination llvm value
 * @param ctx       The llvm traversal context
 */
void bllvm_memcpy(struct LLVMOpaqueValue *from,
                  struct LLVMOpaqueValue *to,
                  struct llvm_traversal_ctx *ctx);
/**
 * Memcpy a pointer value for specific number of bytes
 *
 * Just like @ref bllvm_memcpy() but specify the number of bytes to copy
 */
void bllvm_memcpyn(struct LLVMOpaqueValue *from,
                   struct LLVMOpaqueValue *to,
                   uint32_t bytes,
                   struct llvm_traversal_ctx *ctx);

/**
 * A no-op. Calls intrinsic llvm.donothing()
 */
void bllvm_nop(struct llvm_traversal_ctx *ctx);

/**
 * Convenience function to perform a GEP to an element of a struct
 *
 * @param ptr            An LLVMValueRef of the struct pointer
 * @param member_num     The index to the struct member to perform the GEP for
 * @param ctx            The llvm context
 * @return               GetElementPointer for the given parameters
 */
struct LLVMOpaqueValue *bllvm_gep_to_struct(struct LLVMOpaqueValue *ptr,
                                            unsigned int member_num,
                                            struct llvm_traversal_ctx *ctx);

unsigned long long  bllvm_type_storagesize(
    struct LLVMOpaqueTargetData *tdata,
    struct LLVMOpaqueType *type,
    struct llvm_traversal_ctx *ctx
);
#endif
