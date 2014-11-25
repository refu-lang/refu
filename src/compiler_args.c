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
    _args.output = NULL;
    rf_string_init(&_args.input, "");
    rf_stringx_init_buff(&_args.buff, 128, "");
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

static inline bool check_string_value(struct compiler_args *args, char* s)
{
    uint64_t val;
    if (!rf_stringx_assign_unsafe_nnt(&args->buff, s, strlen(s))) {
        ERROR("Could not assign input argument to string");
        return false;
    }

    if (!rf_string_to_uint_dec(&args->buff, &val, NULL)) {
        ERROR("Verbose level argument is not a number: %s", s);
        return false;
    }
    args->verbose_level = val;
    return true;
}

static bool check_verbose_level(struct compiler_args *args,
                                int* i, int argc, char** argv,
                                bool* consumed)
{
    if (strcmp(argv[*i], "-v") == 0 ||
        strcmp(argv[*i], "--verbose-level") == 0) {

        *i = *i + 1;
        if (argc >= *i) {
            *consumed = true;
            return check_string_value(args, argv[*i]);
        } else {
            ERROR("A number should follow the verbose argument");
            return false;
        }
    }
    if (strstr(argv[*i], "--verbose-level=")) {
        *consumed = true;
        return check_string_value(args, argv[*i]+16);
    }
    return false;
}

static void check_repl(int* i, int argc, char** argv, bool* consumed)
{
    if(strcmp(argv[*i], "-r") == 0 ||
       strcmp(argv[*i], "--repl") == 0)
    {
        *consumed = true;
        _args.repl = true;
        _args.backend_connection = BACKEND_INTERPRETER;
    }
}

static bool check_output_name(int* i, int argc, char** argv, bool* consumed)
{
    if(strcmp(argv[*i], "-o") == 0 ||
       strcmp(argv[*i], "--output") == 0)
    {
        *i = *i + 1;
        if (argc >= *i) {
            *consumed = true;
            _args.output = rf_string_create(argv[*i]);
            if (!_args.output) {
                ERROR("Internal error while consuming the input file argument");
                return false;
            }
        } else {
            ERROR("A filename should follow the output argument");
            return false;
        }
    }
    return true;
}

struct compiler_args *compiler_args_parse(int argc, char** argv)
{
    bool consumed;
    bool ok;
    int i;
    for (i = 1; i < argc; i++) {

        consumed = false;
        if(!check_backend(&i, argc, argv, &consumed))
        {
            ERROR("Error while consuming the backend "
                        "connection argument");
            printf(help_message);
            return NULL;
        }

        ok = check_verbose_level(&_args, &i, argc, argv, &consumed);
        if (consumed) {
            if (!ok) {
                return NULL;
            }
            INFO(4, "Verbosity level set to %u", _args.verbose_level);
        }

        check_repl(&i, argc, argv, &consumed);
        if (consumed) {
            continue;
        }

        if (!check_output_name(&i, argc, argv, &consumed)) {
            return NULL;
        }
        if (consumed) {
            continue;
        }

        /* if we get here the argument should be a file */
        if(!rf_string_assign(&_args.input, RFS_(argv[i]))) {
            ERROR("Internal error while consuming the input file argument");
            return NULL;
        }
        if(!rf_system_file_exists(&_args.input)) {
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

