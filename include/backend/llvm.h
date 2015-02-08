#ifndef LFR_BACKEND_LLVM_H
#define LFR_BACKEND_LLVM_H

#include <stdbool.h>

struct analyzer;
struct compiler_args;
struct rir;
struct rir_module;


/**
 * Processes the Refu IR module and creates the backend code
 *
 * @param module        The ir module to process. Is also freed after processing
 * @param rir           The ir handler. Need from some data, lke the string tables
 * @param args          The arguments given to the compiler
 *
 * @return              true in succes and false for failure
 */
bool backend_llvm_generate(struct rir_module *module, struct rir *rir,
                           struct compiler_args *args);

#endif
