#include <stdio.h>
#include <refu.h>

#include <compiler_args.h>
#include <front_ctx.h>
#include <backend/llvm.h>
#include <Utils/log.h>

struct ast_node;

int main(int argc,char** argv)
{
    int rc = 0;
    struct compiler_args *args;
    struct front_ctx front;
    struct analyzer *analyzer;
    struct RFstringx err_buff;

    // initialize Refu library
    rf_init("refuclib.log", 0, LOG_DEBUG);

    // initialize an error buffer string
    if (!rf_stringx_init_buff(&err_buff, 1024, "")) {
        ERROR("Internal error");
        rc = 1;
        goto err;
    }

    //parse the arguments given to the compiler
    compiler_args_modinit();
    args = compiler_args_parse(argc, argv);
    if(!args) {
        ERROR("Failure at argument parsing");
        rc = 1;
        goto err;
    }

    if (rf_string_is_empty(&args->input)) {
        ERROR("No input file provided");
        rc = 1;
        goto err;
    }

    if (!front_ctx_init(&front, args)) {
        ERROR("Failure at frontend initialization");
        rc = 1;
        goto err;
    }

    analyzer = front_ctx_process(&front);
    if (!analyzer) {
        ERROR("Failure to parse and analyze the input");
        rc = 1;

        // for now temporarily just dump all messages in the info context
        // TODO: fix
        if (!info_ctx_get_messages_fmt(front.info, MESSAGE_ANY, &err_buff)) {
            RF_ERROR("Could not retrieve messages from the info context");
            goto err;
        }
        printf(RF_STR_PF_FMT, RF_STR_PF_ARG(&err_buff));
        goto err;
    }
    RF_DEBUG("input file parsed succesfully\n");

    backend_llvm_generate(analyzer, args);

err:
    rf_deinit();
    return rc;
}

