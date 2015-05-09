#include <compiler_args.h>

#include <info/info.h>
#include <string.h>

#include <RFsystem.h>
#include <Utils/memory.h>
#include <argtable/argtable3.h>

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

// Convenience macro to help with the argtable creation since we actually keep
// all of the arg_xxx structs as members of the compiler_args object
#define CREATE_LOCAL_ARGTABLE(_ca)              \
    void *argtable[] = {                        \
        (_ca)->help,                            \
        (_ca)->version,                         \
        (_ca)->verbosity,                       \
        (_ca)->backend,                         \
        (_ca)->backend_debug,                   \
        (_ca)->positional_file,                 \
        (_ca)->end                              \
    }                                           \

bool compiler_args_init(struct compiler_args *a)
{
    a->help = arg_litn(NULL, "help", 0, 1, "display this help and exit");
    a->version = arg_litn(NULL, "version", 0, 1, "display version info and exit");
    a->verbosity = arg_int0("v", "verbose-level", "1-4", "Set compiler verbosity level");
    a->backend = arg_rex0(NULL, "backend", "GCC|LLVM", NULL, 0, "The backend connection the refu compiler will user");
    a->backend_debug = arg_litn(NULL, "backend-debug", 0, 1, "If given then some debug information about the backend code will be printed");
    a->positional_file = arg_filen(NULL, NULL, "<file>", 0, 1, "input files");
    a->end = arg_end(20);

    // set default values
    a->verbosity->ival[0] = VERBOSE_LEVEL_DEFAULT;

    rf_string_init(&a->input, "");
    rf_stringx_init_buff(&a->buff, 128, "");

    // old stuff
    a->repl = false;
    a->output = NULL;

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
    CREATE_LOCAL_ARGTABLE(args);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}

void compiler_args_destroy(struct compiler_args *args)
{
    compiler_args_deinit(args);
    free(args);
}


bool compiler_args_parse(struct compiler_args *args, int argc, char** argv)
{
    int nerrors;
    CREATE_LOCAL_ARGTABLE(args);
    nerrors = arg_parse(argc, argv, argtable);

    if (nerrors != 0) {
        arg_print_errors(stdout, args->end, "refu");
        exit(1);
    }

    if (args->positional_file->count > 0) {
        // for now take only the first file
        if (!rf_string_assignv(&args->input, "%s", args->positional_file->filename[0])) {
            ERROR("Internal error while consuming the input file argument");
            return false;
        }
        if (!rf_system_file_exists(&args->input)) {
            ERROR("File \""RF_STR_PF_FMT"\" does not exist",
                  RF_STR_PF_ARG(&args->input));
            return false;
        }
    } else if (!compiler_args_help_is_requested(args)) {
        // TODO: Maybe use stdin as input if no file is provided ?
        ERROR("No input file was provided");
        return false;
    }

    return true;
}

bool compiler_args_check_and_display_help(struct compiler_args *args)
{
    CREATE_LOCAL_ARGTABLE(args);
    if (args->help->count > 0) {
        arg_print_glossary(stdout, argtable, " %-55s %s\n");
        return true;
    }
    if (args->version->count > 0) {
        printf("%s", version_message);
        return true;
    }
    return false;
}

bool compiler_args_help_is_requested(struct compiler_args *args)
{
    return args->help->count > 0 || args->version->count > 0;
}

bool compiler_args_print_backend_debug(struct compiler_args *args)
{
    return args->backend_debug->count > 0;
}

i_INLINE_INS struct RFstring *compiler_args_get_output(struct compiler_args *args);
