#ifndef LFR_BACKEND_LLVM_GLOBALS_H
#define LFR_BACKEND_LLVM_GLOBALS_H

#include <stdbool.h>
#include <stdint.h>

struct llvm_traversal_ctx;
struct rir;
struct RFstring;
struct LLVMOpaqueValue;

bool bllvm_create_globals(struct llvm_traversal_ctx *ctx);
bool bllvm_create_module_types(struct rir *r, struct llvm_traversal_ctx *ctx);
bool bllvm_create_module_globals(struct rir* r, struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueValue *bllvm_create_global_const_string(
    const struct RFstring *string_name,
    const struct RFstring *string_val,
    struct llvm_traversal_ctx *ctx
);
struct LLVMOpaqueValue *bllvm_create_global_const_string_with_hash(
    const struct RFstring *string_name,
    const struct RFstring *string_val,
    uint32_t hash,
    struct llvm_traversal_ctx *ctx
);

struct LLVMOpaqueValue *bllvm_literal_to_global_string(
    const struct RFstring *lit,
    struct llvm_traversal_ctx *ctx
);

/**
 * @returns the global string for "true" or "false"
 */
struct LLVMOpaqueValue *bllvm_get_boolean_str(
    bool boolean,
    struct llvm_traversal_ctx *ctx
);
#endif
