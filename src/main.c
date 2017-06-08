#include <stdio.h>
#include <rfbase/refu.h>
#include <compiler.h>

int main(int argc, char **argv)
{
    struct compiler *compiler = compiler_create_with_args(
        LOG_TARGET_STDOUT, // rflog print to stdout
        true,              // use stdlib
        argc,
        argv
    );
    if (!compiler) {
        return 1;
    }

    if (compiler_help_requested(compiler)) {
        // DO not continue any further and do not deinit the compiler since
        // initialization did not fully conclude
        return 0;
    }

    if (!compiler_process(compiler)) {
        compiler_print_errors(compiler);
        return 1; // don't bother freeing stuff, just exit with error
    }

    compiler_destroy(compiler);
    return 0;
}
