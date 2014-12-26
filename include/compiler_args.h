/*
 * An argument parsing module. Could have used getopt()
 * but I was feeling adventurous and thinking of an eventual port
 * to windows
 */

#ifndef LFR_COMPILER_ARGS_H
#define LFR_COMPILER_ARGS_H

#include <RFstring.h>
#include <stdbool.h>

struct compiler_args {
    int backend_connection;
    unsigned verbose_level;
    bool repl;
    struct RFstring input;
    struct RFstring *output;
    struct RFstringx buff;
};

struct compiler_args *compiler_args_parse(int argc, char** argv);

struct compiler_args *compiler_args_get();

/** Initializes the compiler_arguments to their defaults */
void compiler_args_modinit();


/* -- Getters -- */
i_INLINE_DECL struct RFstring *compiler_args_get_output(struct compiler_args *args)
{
    if (args->output) {
        return args->output;
    }

    return &args->input;
}

#endif//include guards end
