#include <serializer/serializer.h>

#include <Utils/memory.h>
#include <analyzer/analyzer.h>
#include <compiler_args.h>
#include "astprinter.h"

bool serializer_init(struct serializer *sr, struct compiler_args *args)
{
    sr->args = args;
    return true;
}

struct serializer *serializer_create(struct compiler_args *args)
{
    struct serializer *sr;
    RF_MALLOC(sr, sizeof(*sr), return NULL);

    if (!serializer_init(sr, args)) {
        free(sr);
        return NULL;
    }

    return sr;
}

void serializer_destroy(struct serializer *sr)
{
    free(sr);
}

bool serializer_process(struct serializer *sr,
                        const struct ast_node *root,
                        const struct inpfile *f)
{
    if (compiler_args_output_ast(sr->args)) {
        // for now just put it stdout, we will configure via compiler-args
        if (!ast_output_to_file(root, stdout, f)) {
            return SERC_FAILURE;
        }
        return SERC_SUCCESS_EXIT;
    }
    return SERC_SUCCESS_CONTINUE;
}
