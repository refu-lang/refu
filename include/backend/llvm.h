#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct RFilist_head;
struct compiler_args;


/**
 * Generate the backend code with llvm
 *
 * @param fronts_list   A list of front_ctxs to try and compile
 * @param args          The arguments given to the compiler
 *
 * @return              true in succes and false for failure
 */
bool bllvm_generate(struct RFilist_head *fronts, struct compiler_args *args);

#endif
