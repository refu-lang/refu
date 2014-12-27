#ifndef LFR_REFU_COMPILER_H
#define LFR_REFU_COMPILER_H

#include <stdbool.h>
#include <String/rf_str_core.h>

struct compiler_args;

struct compiler {
    //! An error buffer for the compiler
    struct RFstringx err_buff;
    //! The object holding the compiler arguments
    struct compiler_args *args;
    //! The compiler's front end context
    struct front_ctx *front;
};

bool compiler_init(struct compiler *c);

bool compiler_init_with_args(struct compiler *c, int argc, char **argv);

bool compiler_process(struct compiler *c);

void compiler_deinit(struct compiler *c);

#endif
