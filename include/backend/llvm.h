#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct analyzer;
struct compiler_args;


bool backend_llvm_generate(struct analyzer *analyzer, struct compiler_args *args);

#endif
