#include <stdio.h>
#include <refu.h>

#include <compiler_args.h>
#include <parser/parser.h>
#include <ast/ast.h>
#include <info/info.h>

int main(int argc,char** argv)
{
    struct parser_ctx *parser;
    struct compiler_args *args;
    //initialize Refu library
    rf_init("refuclib.log", 0, LOG_DEBUG);


    //parse the arguments given to the compiler
    compiler_args_modinit();
    args = compiler_args_parse(argc, argv);
    if(!args) {
        ERROR("Failure at argument parsing");
        return -1;
    }

    parser = parser_new();
    if (!parser) {
        ERROR("Failure at parser initialization");
        return -1;
    }

    if (!parser_process_file(parser, &args->input)) {
        parser_flush_messages(parser);
        ERROR("Failure at file parsing");
        return -1;
    }
    parser_flush_messages(parser);
    ast_print(parser->current_file->root, 0, 0);
    
    return 0;
}

