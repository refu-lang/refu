#include "compiler.h"

#include <rfbase/refu.h>
#include <rfbase/utils/memory.h>
#include <rfbase/string/corex.h>
#include <rfbase/string/traversalx.h>
#include <rfbase/system/system.h>

#include <utils/string_set.h>
#include <info/info.h>
#include <types/type_comparisons.h>
#include <module.h>
#include <compiler_args.h>
#include <ast/ast.h>
#include <front_ctx.h>
#include <ownership/ownership.h>
#include <serializer/serializer.h>
#include <backend/llvm.h>
#include <ir/rir.h>
#include <ir/rir_utils.h>
#include <ir/parser/rirparser.h>

struct rir_module;
static struct compiler *g_compiler_instance = NULL;

bool compiler_init(struct compiler *c, int rf_logtype, bool with_stdlib)
{
    RF_STRUCT_ZERO(c);

    // initialize Refu library
    rf_init(
        rf_logtype,
        "refu.log",
        LOG_WARNING,
        RF_DEFAULT_TS_MBUFF_INITIAL_SIZE,
        RF_DEFAULT_TS_SBUFF_INITIAL_SIZE
    );

    darray_init(c->modules);

    // initialize an error buffer string
    if (!rf_stringx_init_buff(&c->err_buff, 1024, "")) {
        return false;
    }
    // initialize the type comparison thread local context
    if (!typecmp_ctx_init()) {
        return false;
    }

    if (!(c->args = compiler_args_create())) {
        return false;
    }

    if (!(c->serializer = serializer_create(c->args))) {
        return false;
    }
    rf_ilist_head_init(&c->front_ctxs);
    c->use_stdlib = with_stdlib;

    // locate LLVM's llc executable in the user's system
    static const struct RFstring llc_s = RF_STRING_STATIC_INIT("llc");
    static const struct RFstring llc_38s = RF_STRING_STATIC_INIT("llc-3.8");
    static const struct RFstring llc_37s = RF_STRING_STATIC_INIT("llc-3.7");
    if (!rf_system_file_in_path(&llc_s, &c->llc_exec_path) &&
        !rf_system_file_in_path(&llc_38s, &c->llc_exec_path) &&
        !rf_system_file_in_path(&llc_37s, &c->llc_exec_path)) {

        RF_ERROR("Could not find LLVM's llc in the operating system");
        return false;
    }

    return true;
}

struct compiler *compiler_alloc()
{
    struct compiler *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    g_compiler_instance = ret;
    return ret;
}

struct compiler *compiler_create(int rf_logtype, bool with_stdlib)
{

    RF_ASSERT_OR_EXIT(!g_compiler_instance, 
                      "compiler_create() was called a second time");
    struct compiler *compiler = compiler_alloc();
    return compiler_init(compiler, rf_logtype, with_stdlib) ? compiler : NULL;
}

static bool compiler_init_with_args(struct compiler *c, int rf_logtype, bool with_stdlib, int argc, char **argv)
{
    if (!compiler_init(c, rf_logtype, with_stdlib)) {
        return false;
    }

    return compiler_pass_args(argc, argv);
}

struct compiler *compiler_create_with_args(int rf_logtype, bool with_stdlib, int argc, char **argv)
{
    RF_ASSERT_OR_EXIT(
        !g_compiler_instance,
        "compiler_create() was called a second time"
    );
    struct compiler *ret = compiler_alloc();
    if (!compiler_init_with_args(ret, rf_logtype, with_stdlib, argc, argv)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct compiler *compiler_instance_get()
{
    return g_compiler_instance;
}

struct front_ctx *compiler_new_front(
    struct compiler *c,
    enum rir_pos codepath,
    const struct RFstring *file_name,
    const struct RFstring *source
)
{
    struct front_ctx *front;
    if (!(front = front_ctx_create(c->args, codepath, file_name, source))) {
        RF_ERROR("Failure at frontend context initialization");
        return NULL;
    }

    rf_ilist_add(&c->front_ctxs, &front->ln);
    return front;
}

static void compiler_deinit(struct compiler *c)
{
    struct front_ctx *front;
    struct front_ctx *tmp;
    struct module **mod;
    rf_ilist_for_each_safe(&c->front_ctxs, front, tmp, ln) {
        front_ctx_destroy(front);
    }
    
    darray_foreach(mod, c->modules) {
        module_destroy(*mod);
    }
    // free rir utilities created at rir_process for all rir modules
    darray_free(c->modules);
    rir_utils_destroy();


    serializer_destroy(c->serializer);
    compiler_args_destroy(c->args);
    typecmp_ctx_deinit();
    rf_stringx_deinit(&c->err_buff);
    rf_string_deinit(&c->llc_exec_path);
    rf_deinit();
}

void compiler_destroy(struct compiler *c)
{
    compiler_deinit(c);
    free(c);
    g_compiler_instance = NULL;
}

bool compiler_set_main(struct front_ctx *front)
{
    struct compiler *c = g_compiler_instance;
    if (c->main_front) {
        // TODO: maybe change this? Not very elegant. Could somehow try to provide main's location
        // set the error location as the beginning of the file
        struct inplocation_mark start = LOCMARK_INIT(front->file, 0, 0);
        struct inplocation_mark end = LOCMARK_INIT(front->file, 0, 0);
        i_info_ctx_add_msg(
            front->info,
            MESSAGE_SEMANTIC_ERROR,
            &start,
            &end,
            "Multiple definition of main() detected in \""RFS_PF"\". "
            "Previous definition was in \""RFS_PF"\".",
            RFS_PA(front_ctx_filename(front)),
            RFS_PA(front_ctx_filename(c->main_front))
        );
        return false;
    }
    front->is_main = true;
    c->main_front = front;
    return true;
}

bool compiler_pass_args(int argc, char **argv)
{
    struct compiler *c = g_compiler_instance;
    if (!compiler_args_parse(c->args, argc, argv)) {
        return false;
    }

    // do not proceed any further if we got request for help
    if (compiler_args_help_is_requested(c->args)) {
        return true;
    }

    if (!compiler_args_have_input(c->args)) {
        ERROR("No input files were given");
        return true;
    }

    // add all input files as new fronts
    unsigned i;
    for (i = 0; i < compiler_args_get_input_num(c->args); ++i) {
        if (!compiler_new_front(c, RIRPOS_NONE, &c->args->input_files[i], NULL)) {
            return false;
        }
    }
    return true;
}

struct module *compiler_module_get(const struct RFstring *name)
{
    struct module **mod;
    struct compiler *c = g_compiler_instance;
    darray_foreach(mod, c->modules) {
        if (rf_string_equal(module_name(*mod), name)) {
            return *mod;
        }
    }
    return NULL;
}

static unsigned compiler_get_module_index(struct module *m)
{
    struct module **mod;
    unsigned found_index = 0;
    bool found = false;
    darray_foreach(mod, compiler_instance_get()->modules) {
        if (*mod == m) {
            found = true;
            break;
        }
        found_index ++;
    }
    RF_ASSERT(found, "A module must have been found at this point");
    return found_index;
}


// Algorithm for topological sorting as seen here: https://en.wikipedia.org/wiki/Topological_sorting
// One main difference is the direction of the edges. Each module has edges towards
// what it depends on and not what module depends on it. This is the reason we add
// to the tail of the linked list instead of the head as the algorithm states.

enum mark_states {
    STATE_UNMARKED = 0,
    STATE_TEMP_MARK,
    STATE_MARKED
};

static bool compiler_visit_unmarked_module(struct module *m, unsigned i, enum mark_states *marks)
{
    if (marks[i] == STATE_MARKED) {
        return true;
    }

    // not a DAG, cyclic dependency detected
    if (marks[i] == STATE_TEMP_MARK) {
        // TODO: reporting the offending mdoule can be improved? As it currently
        // stands this reports any module in the cycle path
        i_info_ctx_add_msg(
            m->front->info,
            MESSAGE_SEMANTIC_ERROR,
            ast_node_startmark(m->node),
            ast_node_endmark(m->node),
            "Cyclic dependency around module \""RFS_PF"\" detected.",
            RFS_PA(module_name(m))
        );
        return false;
    }

    marks[i] = STATE_TEMP_MARK;
    struct module **dependency;
    darray_foreach(dependency, m->dependencies) {
        // find index of dependency in marks
        unsigned found_index = compiler_get_module_index(*dependency);
        // visit the module
        if (!compiler_visit_unmarked_module(*dependency, found_index, marks)) {
            return false;
        }
    }
    marks[i] = STATE_MARKED;

    // add module m to the end of the sorted list.
    rf_ilist_add_tail(&compiler_instance_get()->sorted_modules, &m->ln);
    return true;
}

static bool compiler_resolve_dependencies()
{
    struct compiler *c = g_compiler_instance;
    struct module **mod;
    rf_ilist_head_init(&c->sorted_modules);
    // topologically sort the dependency DAG
    unsigned int i;
    enum mark_states *marks;
    RF_CALLOC(marks, darray_size(c->modules), sizeof(*marks), return false);

    bool unmarked_found = true;
    while (unmarked_found) {
        i = 0;
        unmarked_found = false;
        darray_foreach(mod, c->modules) {
            // if module is unmarked
            if (marks[i] != STATE_MARKED) {
                unmarked_found = true;
                if (!compiler_visit_unmarked_module(*mod, i, marks)) {
                    free(marks);
                    return false;
                }
            }
            i++;
        }
    }

    free(marks);
    return true;
}

bool compiler_preprocess_fronts()
{
    struct compiler *c = g_compiler_instance;
    struct front_ctx *front;
    bool ret = false;
    // make sure all files are parsed
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (!front_ctx_parse(front)) {
            return false;
        }
    }

    // determine the dependencies of all the modules
    struct rf_objset_string mod_names_set;
    rf_objset_init(&mod_names_set, string);
    struct module **mod;
    darray_foreach(mod, c->modules) {
        const struct RFstring *mname = module_name(*mod);
        // First check if a module with same name is in the string set
        if (rf_objset_get(&mod_names_set, string, mname)) {
            i_info_ctx_add_msg(
                (*mod)->front->info,   
                MESSAGE_SEMANTIC_ERROR,
                ast_node_startmark((*mod)->node),
                ast_node_endmark((*mod)->node),
                "Module \""RFS_PF"\" already declared",
                RFS_PA(mname)
            );
            goto end_free_names;
        }
        // add this module's name to the set
        rf_objset_add(&mod_names_set, string, mname);
        
        if (!module_determine_dependencies(*mod, c->use_stdlib)) {
            // syntactic errors should have been added inside the above function
            goto end_free_names;
        }
    }

    // resolve dependencies and figure out analysis order
    if (!compiler_resolve_dependencies()) {
        // cyclic module dependency (error already reported in the function)
        goto end_free_names;
    }
    // success
    ret = true;

end_free_names:
    rf_objset_clear(&mod_names_set);
    return ret;
}

bool compiler_analyze()
{
    struct compiler *c = g_compiler_instance;
    // analyze the modules that came from AST source parsing in the topologically sorted order
    struct module *mod;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        if (module_rir_codepath(mod) == RIRPOS_AST) {
            if (!module_analyze(mod)) {
                return false;
            }
        }
    }
    return true;
}

bool compiler_process()
{
    struct compiler *c = g_compiler_instance;

    // add the standard library to the front contexts
    struct front_ctx *stdlib_front;
    static const struct RFstring stdlib = RF_STRING_STATIC_INIT(RF_LANG_CORE_ROOT"/stdlib/io.rf");
    if (!(stdlib_front = compiler_new_front(c, RIRPOS_AST, &stdlib, NULL))) {
        RF_ERROR("Failed to add standard library to the front_ctxs");
        return false;
    }

    if (!compiler_preprocess_fronts()) {
        return false;
    }
    
    if (!compiler_analyze()) {
        return false;
    }

#if 0 // don't call serializer at all for now
    enum serializer_rc rc = serializer_process(c->serializer, front);
    if (rc == SERC_SUCCESS_EXIT || rc == SERC_SUCCESS_EXIT) {
        return rc;
    }
#endif

    // create the Refu Intermediate Format
    if (!compiler_create_rir(c)) {
        RF_ERROR("Failed to process the Refu IR");
        return false;
    }

    // perform ownership analysis on the created IR
    if (!ownership_pass(c)) {
        RF_ERROR("Failed at ownership pass");
        return false;
    }

    if (compiler_args_print_rir(c->args)) {
        // print the RIR string representation and quit. Move somewhere else..?
        if (!rir_print(c)) {
            RF_ERROR("Failed to print the Refu IR");
            return false;
        }
        // if we printed succesfully quit
        return true;
    }

    if (!bllvm_generate(&c->modules, c->args)) {
        RF_ERROR("Failed to create the LLVM IR from the Refu IR");
        return false;
    }

    return true;
}

bool compiler_help_requested(struct compiler *c)
{
    return compiler_args_check_and_display_help(c->args);
}

struct RFstringx *compiler_get_errors_from_front(struct compiler *c, struct info_ctx *info)
{
    RF_ASSERT(rf_string_is_empty(&c->err_buff),
              "compiler error buffer string should be empty before");
    (info_ctx_get_messages_fmt(info, MESSAGE_ANY, &c->err_buff));
    // if anything got added move internal string pointer to beginning
    rf_stringx_reset(&c->err_buff);
    return rf_string_is_empty(&c->err_buff) ? NULL : &c->err_buff;
}

struct RFstringx *compiler_get_errors(struct compiler *c)
{
    struct front_ctx *front;
    RF_ASSERT(rf_string_is_empty(&c->err_buff),
              "compiler error buffer string should be empty before");
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        // append error messages to the error buffer string
        (info_ctx_get_messages_fmt(front->info, MESSAGE_ANY, &c->err_buff));
    }
    // if anything got added move internal string pointer to beginning
    rf_stringx_reset(&c->err_buff);
    return rf_string_is_empty(&c->err_buff) ? NULL : &c->err_buff;
}

static inline void compiler_print_errors_common(struct RFstringx *str, struct compiler *c)
{
    if (str) {
        RFS_PUSH();
        printf(RFS_PF, RFS_PA(str));
        RFS_POP();
    } else {
        printf("Unknown compiler error detected.\n");
    }
    rf_stringx_clear(&c->err_buff);
}

void compiler_print_errors(struct compiler *c)
{
    struct RFstringx *str = compiler_get_errors(c);
    compiler_print_errors_common(str, c);
}

void compiler_print_errors_from_front(struct compiler *c, struct info_ctx *info)
{
    struct RFstringx *str = compiler_get_errors_from_front(c, info);
    compiler_print_errors_common(str, c);
}
