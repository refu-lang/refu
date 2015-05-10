/*
 * An argument parsing module. Could have used getopt()
 * but I was feeling adventurous and thinking of an eventual port
 * to windows
 */

#ifndef LFR_COMPILER_ARGS_H
#define LFR_COMPILER_ARGS_H

#include <RFstring.h>
#include <stdbool.h>


struct arg_file;
struct arg_lit;
struct arg_int;
struct arg_rex;
struct arg_end;

struct compiler_args {
    struct RFstring input;
    struct RFstring *output;
    struct RFstringx buff;

    /* -- argtable related members -- */
    struct arg_lit *help;
    struct arg_lit *version;
    struct arg_int *verbosity;
    // does not really do anything but keep it as example of regex argtable arg
    struct arg_rex *backend;
    struct arg_lit *backend_debug;
    struct arg_str *output_name;
    struct arg_file *positional_file;
    struct arg_end *end;
};

bool compiler_args_init(struct compiler_args *args);
struct compiler_args *compiler_args_create();

void compiler_args_deinit(struct compiler_args *args);
void compiler_args_destroy(struct compiler_args *args);

bool compiler_args_parse(struct compiler_args *args, int argc, char **argv);

/**
 * Check if any sort of help argument was given and display it if it was.
 * Help arguments are help, version e.t.c.
 *
 * @param args      The compiler args object
 * @return          True if a help argument was given and we need to exit after
 *                  displaying it. False otherwise
 */
bool compiler_args_check_and_display_help(const struct compiler_args *args);

/**
 * Returns true if any help argument was requested. Does not display anything
 */
bool compiler_args_help_is_requested(const struct compiler_args *args);

/**
 * Should we print backend llvm debug information?
 */
bool compiler_args_print_backend_debug(const struct compiler_args *args);

/**
 * Get the name of the output file that should be generated
 */
struct RFstring *compiler_args_get_output(const struct compiler_args *args);

#endif//include guards end
