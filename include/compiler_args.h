/*
 * An argument parsing module. Could have used getopt()
 * but I was feeling adventurous and thinking of an eventual port
 * to windows
 */

#ifndef LFR_COMPILER_ARGS_H
#define LFR_COMPILER_ARGS_H

#include <RFstring.h>
#include <stdbool.h>

enum help_requested_choices {
    HELP_ARGS = 0,
    HELP_VERSION,
    HELP_NONE
};

struct compiler_args {
    int backend_connection;
    unsigned verbose_level;
    bool repl;
    enum help_requested_choices help_requested;
    struct RFstring input;
    struct RFstring *output;
    struct RFstringx buff;
};

bool compiler_args_init(struct compiler_args *args);
struct compiler_args *compiler_args_create();

void compiler_args_deinit(struct compiler_args *args);
void compiler_args_destroy(struct compiler_args *args);

bool compiler_args_parse(struct compiler_args *args, int argc, char** argv);

void compiler_args_print_help();
void compiler_args_print_version();

/* -- Getters -- */
i_INLINE_DECL struct RFstring *compiler_args_get_output(struct compiler_args *args)
{
    if (args->output) {
        return args->output;
    }

    return &args->input;
}

#endif//include guards end
