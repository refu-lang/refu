#ifndef LFR_REFU_COMPILER_H
#define LFR_REFU_COMPILER_H

#include <stdbool.h>
#include <String/rf_str_core.h>

struct compiler_args;
struct serializer;
struct rir;

struct compiler {
    //! An error buffer for the compiler
    struct RFstringx err_buff;
    //! The object holding the compiler arguments
    struct compiler_args *args;
    //! The compiler's front end context
    struct front_ctx *front;
    //! The intermediate representation of the code, created after the analysis
    //! of the code.
    struct rir *ir;
    //! The serializer that can serialize the Intermediate Representation to a file
    //! TODO: Maybe just lose this in favour of a to_file() in the IR itself?
    struct serializer *serializer;
};

bool compiler_init(struct compiler *c);

void compiler_deinit(struct compiler *c);

bool compiler_init_with_args(struct compiler *c, int argc, char **argv);

bool compiler_pass_args(struct compiler *c, int argc, char **argv);

bool compiler_process(struct compiler *c);

//! Query compiler's argument and if help was requested, print help message and
//! return true. If true, program should exit succesfully
bool compiler_help_requested(struct compiler *c);

#endif
