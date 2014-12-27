#include "compiler.h"

#include <refu.h>
#include <Utils/memory.h>

#include <info/info.h>
#include <compiler_args.h>
#include <front_ctx.h>
#include <backend/llvm.h>

bool compiler_init(struct compiler *c)
{
    RF_STRUCT_ZERO(c);

    // initialize Refu library
    rf_init("refuclib.log", 0, LOG_DEBUG);
    
    // initialize an error buffer string
    if (!rf_stringx_init_buff(&c->err_buff, 1024, "")) {
        return false;
    }

    if (!(c->args = compiler_args_create())) {
        return false;
    }

    return true;
}

bool compiler_init_with_args(struct compiler *c, int argc, char **argv)
{
    if (!compiler_init(c)) {
        return false;
    }

    if (!compiler_args_parse(c->args, argc, argv)) {
        ERROR("Failed to parse command line arguments");
        return false;
    }

    if (!(c->front = front_ctx_create(c->args))) {
        ERROR("Failure at frontend context initialization");
        return false;
    }

    return true;
}

bool compiler_process(struct compiler *c)
{
    struct analyzer *analyzer = front_ctx_process(c->front);
    if (!analyzer) {
        ERROR("Failure to parse and analyze the input");

        // for now temporarily just dump all messages in the info context
        // TODO: fix
        if (!info_ctx_get_messages_fmt(c->front->info, MESSAGE_ANY, &c->err_buff)) {
            RF_ERROR("Could not retrieve messages from the info context");
            return false;
        }
        printf(RF_STR_PF_FMT, RF_STR_PF_ARG(&c->err_buff));
        return false;
    }
    RF_DEBUG("input file parsed succesfully\n");

    backend_llvm_generate(analyzer, c->args);

    return true;
}

void compiler_deinit(struct compiler *c)
{
    if (c->front) {
        front_ctx_destroy(c->front);
    }
    
    compiler_args_destroy(c->args);
    rf_stringx_deinit(&c->err_buff);
    rf_deinit();
}
