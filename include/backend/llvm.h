#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct analyzer;
struct compiler_args;
struct rir;
struct rir_module;


/**
 * Processes the Refu IR and creates the backend code
 *
 * @param rir           The ir handler. Need from some data, lke the string tables
 * @param args          The arguments given to the compiler
 *
 * @return              true in succes and false for failure
 */
bool bllvm_generate(struct rir *rir, struct compiler_args *args);

#endif
