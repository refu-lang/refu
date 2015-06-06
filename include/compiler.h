#ifndef LFR_REFU_COMPILER_H
#define LFR_REFU_COMPILER_H

#include <stdbool.h>
#include <RFintrusive_list.h>
#include <String/rf_str_core.h>

struct compiler_args;
struct serializer;
struct rir;

struct compiler {
    //! An error buffer for the compiler
    struct RFstringx err_buff;
    //! The object holding the compiler arguments
    struct compiler_args *args;
    //! A list of compiler's front end contexts. One for each file.
    struct RFilist_head front_ctxs;
    //! The serializer deals with data exporting and serialization (if needed)
    //! after the end of a succesfull analysis.
    struct serializer *serializer;
};

bool compiler_init(struct compiler *c);

void compiler_deinit(struct compiler *c);

bool compiler_init_with_args(struct compiler *c, int argc, char **argv);

//! Passes arguments to the compiler and initializes the front end context
bool compiler_pass_args(struct compiler *c, int argc, char **argv);

bool compiler_process(struct compiler *c);

//! Query compiler's argument and if help was requested, print help message and
//! return true. If true, program should exit succesfully
bool compiler_help_requested(struct compiler *c);

#endif
