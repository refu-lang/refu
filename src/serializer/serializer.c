#include <serializer/serializer.h>

#include <Utils/memory.h>
#include <System/rf_system.h>
#include <analyzer/analyzer.h>
#include <front_ctx.h>
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
                        const struct module *mod)
{
    struct RFstring *out_name;
    static const struct RFstring s_stdout = RF_STRING_STATIC_INIT("stdout");
    if (compiler_args_output_ast(sr->args, &out_name)) {
        FILE *f;
        if (rf_string_equal(&s_stdout, out_name)) {
            f = stdout;
        } else {
            f = rf_fopen(out_name, "wb");
            if (!f) {
                return false;
            }
        }
        // for now just put it stdout, we will configure via compiler-args
        if (!ast_output_to_file(mod->node, f, module_get_file(mod))) {
            return SERC_FAILURE;
        }
        if (f != stdout) {
            fclose(f);
        }
        return SERC_SUCCESS_EXIT;
    }
    return SERC_SUCCESS_CONTINUE;
}
