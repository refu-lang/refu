#include <argparser.h>

#if 0
#include <backend.h>
#endif

#include <messaging.h>

#include <string.h>

#include <RFsystem.h>

//temporary to compile
#define BACKEND_DEFAULT 1
#define BACKEND_INTERPRETER 2
#define BACKEND_GCC 3
#define BACKEND_LLVM 4


static compiler_arguments _args;

static const char* help_message = ""
"Usage: refu [options] file\n"
"--backend [GCC|LLVM]\t The backend connection the refu compiler will use\n"
"";

/** Initializes the compiler_arguments to their defaults */
void argparser_modinit()
{
    _args.backend_connection = BACKEND_DEFAULT;
    _args.verbose_level = VERBOSE_LEVEL_DEFAULT;
    _args.repl = false;
    rf_string_init(&_args.input, "");
}

static inline bool compare_backend_values(char* val)
{
    if(strcmp(val, "GCC") == 0)
    {
        _args.backend_connection = BACKEND_GCC;
        return true;
    }
    else if(strcmp(val, "LLVM") == 0)
    {
        _args.backend_connection = BACKEND_LLVM;
        return true;
    }
    return false;
}

static bool check_backend(int* i, int argc, char** argv, bool* consumed)
{
    int len = strlen(argv[*i]);
    if(!strstr(argv[*i], "--backend"))
    {
        return true;
    }
    if(_args.repl)
    {
        print_warning("Specifying both --backend and --repl options is "
                      "not supported. Backnd is automatically chosen as "
                      "interpreter for the REPL.");
        return true;
    }
    /* if there is an equal */
    if(len > 9 && argv[*i][9] == '=')
    {
        return compare_backend_values(argv[*i]+10);
    }
    else if(argc > *i)
    {
        *i = *i +1;
        return compare_backend_values(argv[(*i)+1]);
    }
    return false;
}


/* verbose level must have already been checked by the initialization of the
   compiler output module. Here simply pass it */
static void check_verbose_level(int* i, int argc, char** argv, bool* consumed)
{
    int len = strlen(argv[*i]);
    if(strcmp(argv[*i], "-v") == 0 ||
       strcmp(argv[*i], "--verbose-level") == 0)
    {
        *i = *i + 1;
        *consumed = true;
    }
    if(strstr(argv[*i], "--verbose-level="))
    {
        *consumed = true;
    }
}

static void check_repl(int* i, int argc, char** argv, bool* consumed)
{
    int len = strlen(argv[*i]);
    if(strcmp(argv[*i], "-r") == 0 ||
       strcmp(argv[*i], "--repl") == 0)
    {
        *consumed = true;
        _args.repl = true;
        _args.backend_connection = BACKEND_INTERPRETER;
    }
}

compiler_arguments* argparser_parse(int argc, char** argv)
{
    bool consumed;
    int i;
    for(i = 1; i < argc; i++)
    {
        consumed = false;
        if(!check_backend(&i, argc, argv, &consumed))
        {
            print_error("Error while consuming the backend "
                        "connection argument");
            printf(help_message);
            return NULL;
        }
        check_verbose_level(&i, argc, argv, &consumed);
        check_repl(&i, argc, argv, &consumed);
        if(consumed)
        {
            continue;
        }
        
        /* if we get here the argument should be a file */
        if(!rf_string_assign(&_args.input, RFS_(argv[i])))
        {
            print_error("Internal error while consuming the input "
                        "file argument");
            return NULL;
        }
        if(!rf_system_file_exists(&_args.input))
        {
            print_error("File \"%S\" does not exist", 
                        &_args.input);
            return NULL;
        }
    }
    return &_args;
}

compiler_arguments* argparser_get_args()
{
    return &_args;
}

