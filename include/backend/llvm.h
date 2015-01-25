#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct analyzer;
struct compiler_args;
struct rir_module;


bool backend_llvm_generate(struct rir_module *module, struct compiler_args *args);

#endif
