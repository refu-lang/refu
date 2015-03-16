#ifndef LFR_BACKEND_LLVM_UTILS_H
#define LFR_BACKEND_LLVM_UTILS_H

struct LLVMOpaqueModule;
struct LLVMOpaqueBasicBlock;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

struct llvm_traversal_ctx;

/* -- Some debugging functions to help in sticky situations -- */
void backend_llvm_val_debug(struct LLVMOpaqueValue *v, const char *val_name);
void backend_llvm_type_debug(struct LLVMOpaqueType *t,
                             const char *type_name,
                             struct llvm_traversal_ctx *ctx);
void backend_llvm_mod_debug(struct LLVMOpaqueModule *m, const char *mod_name);

void backend_llvm_assign_to_string(struct LLVMOpaqueValue *string_alloca,
                                   struct LLVMOpaqueValue *length,
                                   struct LLVMOpaqueValue *string_data,
                                   struct llvm_traversal_ctx *ctx);
void backend_llvm_load_from_string(struct LLVMOpaqueValue *string_alloca,
                                   struct LLVMOpaqueValue **length,
                                   struct LLVMOpaqueValue **string_data,
                                   struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *backend_llvm_cast_value_to_type_maybe(
    struct LLVMOpaqueValue *val,
    struct LLVMOpaqueType *type,
    struct llvm_traversal_ctx *ctx);

void backend_llvm_store(struct LLVMOpaqueValue *val, struct LLVMOpaqueValue *ptr,
                        struct llvm_traversal_ctx *ctx);

/**
 * Positions the builder at end of @c block and makes it current
 *
 * @param ctx             The llvm traversal context
 * @param block           The block to enter
 */
void backend_llvm_enter_block(struct llvm_traversal_ctx *ctx,
                              struct LLVMOpaqueBasicBlock *block);
#endif
