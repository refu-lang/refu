/*
 * An argument parsing module. Could have used getopt()
 * but I was feeling adventurous and thinking of an eventual port
 * to windows
 */

#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <RFstring.h>
#include <stdbool.h>

typedef struct compiler_arguments
{
    int backend_connection;
    int verbose_level;
    bool repl;
    struct RFstring input;
}compiler_arguments;

compiler_arguments* argparser_parse(int argc, char** argv);

compiler_arguments* argparser_get_args();

/** Initializes the compiler_arguments to their defaults */
void argparser_modinit();

#endif//include guards end
