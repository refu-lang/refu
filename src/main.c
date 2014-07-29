#include <stdio.h>
#include <refu.h>
#include <messaging.h>

#include <argparser.h>


int main(int argc,char** argv)
{
    compiler_arguments* args;
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
    
    return 0;
}

