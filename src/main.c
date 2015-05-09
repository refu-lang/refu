#include <stdio.h>

#include <compiler.h>

int main(int argc, char **argv)
{
    int rc = 0;
    struct compiler compiler;

    if (!compiler_init_with_args(&compiler, argc, argv)) {
        return 1;
    }

    if (compiler_help_requested(&compiler)) {
        // DO not continue any further and do not deinit the compiler since
        // initialization did not fully conclude
        return 0;
    }

    if (!compiler_process(&compiler)) {
        rc = 1;
        goto end;
    }

end:
    compiler_deinit(&compiler);
    return rc;
}
