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
        (_ca)->output_ast,                      \
        (_ca)->output_name,                     \
        (_ca)->positional_file,                 \
        (_ca)->end                              \
    }                                           \

bool compiler_args_init(struct compiler_args *a)
{
    RF_STRUCT_ZERO(a);
    a->help = arg_litn(NULL, "help", 0, 1, "display this help and exit");
    a->version = arg_litn(NULL, "version", 0, 1, "display version info and exit");
    a->verbosity = arg_int0("v", "verbose-level", "1-4", "Set compiler verbosity level");
    a->backend = arg_rex0(NULL, "backend", "GCC|LLVM", NULL, 0, "The backend connection the refu compiler will user");
    a->output_ast = arg_lit0(NULL, "output-ast", "If given then after analysis state the AST will be output in JSON format");
    a->backend_debug = arg_litn(NULL, "backend-debug", 0, 1, "If given then some debug information about the backend code will be printed");
    a->output_name = arg_str0("o", "output", "name", "output file name. Defaults to input.exe if not given");
    a->positional_file = arg_filen(NULL, NULL, "<file>", 0, 1, "input files");
    a->end = arg_end(20);

    // set default values
    a->verbosity->ival[0] = VERBOSE_LEVEL_DEFAULT;

    rf_string_init(&a->input, "");
    rf_stringx_init_buff(&a->buff, 128, "");

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
    if (args->output != &args->input) {
        rf_string_destroy(args->output);
    }
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

static bool compiler_args_read_file(struct compiler_args *args, const char *name)
{
    if (!rf_string_assignv(&args->input, "%s", name)) {
        ERROR("Internal error while consuming the input file argument");
        return false;
    }
    if (strcmp(name, "stdin") != 0 && !rf_system_file_exists(&args->input)) {
        ERROR("File \""RF_STR_PF_FMT"\" does not exist",
              RF_STR_PF_ARG(&args->input));
        return false;
    }
    return true;
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

    // handle input file name
    if (args->positional_file->count > 0) {
        // for now take only the first file
        if (!compiler_args_read_file(args, args->positional_file->filename[0])) {
            return false;
        }
    } else if (!compiler_args_help_is_requested(args)) {
        // else use stdin as the input method
        if (!compiler_args_read_file(args, "stdin")) {
            return false;
        }
    }

    // handle output file name
    if (args->output_name->count == 0) {
        if (args->positional_file->count == 0) {
            // if no filename is given and we read from stdin
            args->output = rf_string_create("refu_out");
        } else {
            // assume same name as input file
            args->output = &args->input;
        }
    } else {
        args->output = rf_string_create(args->output_name->sval[0]);
    }
    

    return true;
}

bool compiler_args_check_and_display_help(const struct compiler_args *args)
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

bool compiler_args_help_is_requested(const struct compiler_args *args)
{
    return args->help->count > 0 || args->version->count > 0;
}

bool compiler_args_print_backend_debug(const struct compiler_args *args)
{
    return args->backend_debug->count > 0;
}

bool compiler_args_output_ast(const struct compiler_args *args)
{
    return args->output_ast->count > 0;
}

struct RFstring *compiler_args_get_output(const struct compiler_args *args)
{
    return args->output;
}
