#include <stdio.h>

#include <compiler.h>
#include <refu.h>

int main(int argc, char **argv)
{
    int rc = 0;
    struct compiler *compiler = compiler_create_with_args(
        LOG_TARGET_STDOUT, // rflog print to stdout
        true,              // use stdlib
        argc, argv);
    if (!compiler) {
        return 1;
    }

    if (compiler_help_requested(compiler)) {
        // DO not continue any further and do not deinit the compiler since
        // initialization did not fully conclude
        return 0;
    }

    if (!compiler_process(compiler)) {
        rc = 1;
        goto end;
    }

end:
    compiler_destroy(compiler);
    return rc;
}
