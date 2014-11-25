#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct ast_node;
struct compiler_args;

bool backend_llvm_generate(struct ast_node *ast, struct compiler_args *args);

#endif
