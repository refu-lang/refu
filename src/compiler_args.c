#include <compiler_args.h>

#include <info/info.h>
#include <string.h>

#include <RFsystem.h>
#include <Utils/memory.h>

//temporary to compile, TODO: Remove if not used anymore
#define BACKEND_DEFAULT 1
#define BACKEND_INTERPRETER 2
#define BACKEND_GCC 3
#define BACKEND_LLVM 4


#define i_eval(_def) #_def
#define i_str(_def) i_eval(_def)
static const char* version_message = ""
    "Refu language compiler ver"i_str(RF_LANG_MAJOR_VERSION)"." i_str(RF_LANG_MINOR_VERSION) "." i_str(RF_LANG_PATCH_VERSION) "\n";
#undef i_str
#undef i_eval

static const char* help_message = ""
"Usage: refu [options] file\n"
"{-h --help}                        Print this help message\n"
"{-v --verbose-level} [0-4]         Set compiler verbosity level\n"
"{-o --output}        filename      Set the name of the produced output\n"
"--backend            [GCC|LLVM]    The backend connection the refu compiler will use\n"
"--backend-debug                    If given then some debug information about the backend code will be printed\n"
"";

bool compiler_args_init(struct compiler_args *args)
{
    args->backend_connection = BACKEND_DEFAULT;
    args->verbose_level = VERBOSE_LEVEL_DEFAULT;
    args->repl = false;
    args->print_backend_debug = false;
    args->help_requested = HELP_NONE;
    args->output = NULL;
    rf_string_init(&args->input, "");
    rf_stringx_init_buff(&args->buff, 128, "");

    return true;
}

struct compiler_args *compiler_args_create()
{
    struct compiler_args *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!compiler_args_init(ret)) {
        free(ret);
        return NULL;
    }

    return ret;
}

void compiler_args_deinit(struct compiler_args *args)
{
    rf_string_deinit(&args->input);
    rf_stringx_deinit(&args->buff);
}

void compiler_args_destroy(struct compiler_args *args)
{
    compiler_args_deinit(args);
    free(args);
}

static inline bool compiler_args_compare_backend_values(struct compiler_args *args, char* val)
{
    if(strcmp(val, "GCC") == 0) {
        args->backend_connection = BACKEND_GCC;
        return true;
    } else if(strcmp(val, "LLVM") == 0) {
        args->backend_connection = BACKEND_LLVM;
        return true;
    }
    return false;
}

static bool compiler_args_check_backend(struct compiler_args *args, int* i,
                                        int argc, char** argv, bool* consumed)
{
    int len = strlen(argv[*i]);
    if (strcmp(argv[*i], "--backend") != 0) {
        return true;
    }

    if(args->repl) {
        WARN("Specifying both --backend and --repl options is "
             "not supported. Backend is automatically chosen as "
             "interpreter for the REPL.");
        return true;
    }

    /* if there is an equal */
    if (len > 9 && argv[*i][9] == '=') {
        return compiler_args_compare_backend_values(args, argv[*i]+10);
    } else if (argc > *i) {
        *i = *i +1;
        return compiler_args_compare_backend_values(args, argv[(*i)+1]);
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

static bool compiler_args_check_verbosity(struct compiler_args *args,
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

static void compiler_args_check_repl(struct compiler_args *args,int* i,
                                     char **argv, bool* consumed)
{
    if(strcmp(argv[*i], "-r") == 0 ||
       strcmp(argv[*i], "--repl") == 0)
    {
        *consumed = true;
        args->repl = true;
        args->backend_connection = BACKEND_INTERPRETER;
    }
}

static void compiler_args_check_backend_debug(struct compiler_args *args,int* i,
                                              char **argv, bool* consumed)
{
    if(strcmp(argv[*i], "--backend-debug") == 0)
    {
        *consumed = true;
        args->print_backend_debug = true;
    }
}

static bool compiler_args_check_help(struct compiler_args *args, int* i, char **argv)
{
    if(strcmp(argv[*i], "-h") == 0 ||
       strcmp(argv[*i], "--help") == 0) {
        args->help_requested = HELP_ARGS;
        return true;
    }

    if(strcmp(argv[*i], "-v") == 0 ||
       strcmp(argv[*i], "--version") == 0) {
        args->help_requested = HELP_VERSION;
        return true;
    }
    return false;
}

static bool compiler_args_check_output(struct compiler_args *args,int* i,
                                       int argc, char** argv, bool* consumed)
{
    if(strcmp(argv[*i], "-o") == 0 ||
       strcmp(argv[*i], "--output") == 0)
    {
        *i = *i + 1;
        if (argc >= *i) {
            *consumed = true;
            args->output = rf_string_create(argv[*i]);
            if (!args->output) {
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

bool compiler_args_parse(struct compiler_args *args, int argc, char** argv)
{
    bool consumed;
    bool ok;
    int i;
    for (i = 1; i < argc; i++) {
        consumed = false;

        if (compiler_args_check_help(args, &i, argv)) {
            // then stop parsing and compiler will pick up and handle request for help
            return true;
        }

        if (!compiler_args_check_backend(args, &i, argc, argv, &consumed)) {
            ERROR("Error while consuming the backend "
                  "connection argument");
            printf("%s", help_message);
            return false;
        }

        ok = compiler_args_check_verbosity(args, &i, argc, argv, &consumed);
        if (consumed) {
            if (!ok) {
                return false;
            }
        }

        compiler_args_check_backend_debug(args, &i, argv, &consumed);
        if (consumed) {
            continue;
        }

        compiler_args_check_repl(args, &i, argv, &consumed);
        if (consumed) {
            continue;
        }

        if (!compiler_args_check_output(args, &i, argc, argv, &consumed)) {
            return false;
        }
        if (consumed) {
            continue;
        }

        /* if we get here the argument should be a file */
        if (!rf_string_assignv(&args->input, "%s", argv[i])) {
            ERROR("Internal error while consuming the input file argument");
            return false;
        }
        if (!rf_system_file_exists(&args->input)) {
            ERROR("File \""RF_STR_PF_FMT"\" does not exist",
                  RF_STR_PF_ARG(&args->input));
            return false;
        }
    }
    return true;
}

void compiler_args_print_help()
{
    printf("%s", help_message);
}

void compiler_args_print_version()
{
    printf("%s", version_message);
}

i_INLINE_INS struct RFstring *compiler_args_get_output(struct compiler_args *args);
