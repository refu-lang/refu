#include <stdio.h>
#include <refu.h>

#include <compiler_args.h>
#include <front_ctx.h>

int main(int argc,char** argv)
{
    struct compiler_args *args;
    struct front_ctx front;
    //initialize Refu library
    rf_init("refuclib.log", 0, LOG_DEBUG);


    //parse the arguments given to the compiler
    compiler_args_modinit();
    args = compiler_args_parse(argc, argv);
    if(!args) {
        ERROR("Failure at argument parsing");
        return -1;
    }

    if (!front_ctx_init(&front, &args->input)) {
        ERROR("Failure at frontend initialization");
        return -1;
    }

    if (!front_ctx_process(&front)) {
        ERROR("Failure to parse the input");
    } else {
        printf("input file parsed succesfully");
    }
    return 0;
}

