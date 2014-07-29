#include <stdio.h>
#include <refu.h>
#include <messaging.h>

#include <argparser.h>
#include <parser.h>

int main(int argc,char** argv)
{
    compiler_arguments* args;
    struct parser_ctx *parser;
    //initialize Refu library
    rf_init("refuclib.log", 0, LOG_DEBUG);
    //initialize all modules
    argparser_modinit();
    if(!messaging_modinit(argc, argv))
    {
        return -1;
    }

    //parse the arguments given to the compiler
    if(!(args = argparser_parse(argc, argv)))
    {
        print_error("Failure at argument parsing");
        return -1;
    }
    
    parser = parser_new();
    if (!parser) {
        print_error("Failure at parser initialization");
        return -1;
    }

    if (!parser_process_file(parser, &args->input)) {
        print_error("Failure at file parsing");
        return -1;
    }
    
    return 0;
}

