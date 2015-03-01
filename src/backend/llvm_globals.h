#ifndef LFR_BACKEND_LLVM_GLOBALS_H
#define LFR_BACKEND_LLVM_GLOBALS_H

#include <stdbool.h>

struct llvm_traversal_ctx;

bool backend_llvm_create_globals(struct llvm_traversal_ctx *ctx);

#endif
