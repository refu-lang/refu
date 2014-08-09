#include <compiler_args.h>

#if 0
#include <backend.h>
#endif

#include <info/info.h>
#include <string.h>

#include <RFsystem.h>

//temporary to compile
#define BACKEND_DEFAULT 1
#define BACKEND_INTERPRETER 2
#define BACKEND_GCC 3
#define BACKEND_LLVM 4


static struct compiler_args _args;

static const char* help_message = ""
"Usage: refu [options] file\n"
"--backend [GCC|LLVM]\t The backend connection the refu compiler will use\n"
"";

/** Initializes the compiler_arguments to their defaults */
void compiler_args_modinit()
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
        WARN("Specifying both --backend and --repl options is "
             "not supported. Backend is automatically chosen as "
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
#if 0  /* parse verbose level here!!! */
#define VERBOSE_LEVEL_MIN 0
#define VERBOSE_LEVEL_MAX 4
static inline bool check_string_value(struct info_ctx *ctx, char* s)
{
    if (!rf_stringx_assign_unsafe_nnt(&ctx->buff, s, strlen(s))) {
        ERROR("Could not assign input argument to string");
        return false;
    }

    if (!rf_string_to_int(&ctx->buff, &ctx->verbose_level)) {
        ERROR("Verbose level argument is not a number");
        return false;
    }
    return true;
}

static bool parse_args_verbose(struct info_ctx *ctx, int argc, char** argv)
{
    int i;
    ctx->verbose_level = VERBOSE_LEVEL_DEFAULT;
    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-v") == 0 ||
           strcmp(argv[i], "--verbose-level") == 0) {

            if(argc >= i) {
                return check_string_value(ctx, argv[i+1]);
            }
            ERROR("A number should follow the verbose argument");
            return false;
        }

        if(strstr(argv[i], "--verbose-level=")) {
            return check_string_value(ctx, argv[i]+16);
        }
    }
    return true;
}
#endif


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

struct compiler_args *compiler_args_parse(int argc, char** argv)
{
    bool consumed;
    int i;
    for(i = 1; i < argc; i++)
    {
        consumed = false;
        if(!check_backend(&i, argc, argv, &consumed))
        {
            ERROR("Error while consuming the backend "
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
            ERROR("Internal error while consuming the input "
                        "file argument");
            return NULL;
        }
        if(!rf_system_file_exists(&_args.input))
        {
            ERROR("File \""RF_STR_PF_FMT"\" does not exist",
                  RF_STR_PF_ARG(&_args.input));
            return NULL;
        }
    }
    return &_args;
}

struct compiler_args *compiler_args_get()
{
    return &_args;
}

