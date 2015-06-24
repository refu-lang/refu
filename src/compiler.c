#include "compiler.h"

#include <refu.h>
#include <Utils/memory.h>

#include <info/info.h>
#include <types/type_comparisons.h>
#include <module.h>
#include <compiler_args.h>
#include <ast/ast.h>
#include <front_ctx.h>
#include <serializer/serializer.h>
#include <backend/llvm.h>

struct rir_module;
static struct compiler *g_compiler_instance = NULL;

bool compiler_init(struct compiler *c, int rf_logtype)
{
    RF_STRUCT_ZERO(c);

    // initialize Refu library
    rf_init(rf_logtype,
            NULL,
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

    return true;
}

struct compiler *compiler_alloc()
{
    struct compiler *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    // can't use RF_ASSERT() here, comes before rf_init()
    assert(!g_compiler_instance);
    g_compiler_instance = ret;
    return ret;
}

struct compiler *compiler_create(int rf_logtype)
{
    struct compiler *compiler = compiler_alloc();
    return compiler_init(compiler, rf_logtype) ? compiler : NULL;
}

static bool compiler_init_with_args(struct compiler *c, int rf_logtype, int argc, char **argv)
{
    if (!compiler_init(c, rf_logtype)) {
        return false;
    }

    return compiler_pass_args(argc, argv);
}

struct compiler *compiler_create_with_args(int rf_logtype, int argc, char **argv)
{
    struct compiler *ret = compiler_alloc();
    if (!compiler_init_with_args(ret, rf_logtype, argc, argv)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct compiler *compiler_instance_get()
{
    return g_compiler_instance;
}


struct front_ctx *compiler_new_front(struct compiler *c,
                                     const struct RFstring *input_name,
                                     bool is_main)
{
    struct front_ctx *front;
    if (!(front = front_ctx_create(c->args, input_name, is_main))) {
        RF_ERROR("Failure at frontend context initialization");
        return NULL;
    }

    rf_ilist_add(&c->front_ctxs, &front->ln);
    return front;
}

struct front_ctx *compiler_new_front_from_source(struct compiler *c,
                                                 const struct RFstring *name,
                                                 const struct RFstring *source,
                                                 bool is_main)
{
    struct front_ctx *front;
    if (!(front = front_ctx_create_from_source(c->args, name, source, is_main))) {
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
    darray_foreach(mod, c->modules) {
        module_destroy(*mod);
    }
    darray_free(c->modules);
    rf_ilist_for_each_safe(&c->front_ctxs, front, tmp, ln) {
        front_ctx_destroy(front);
    }

    serializer_destroy(c->serializer);
    compiler_args_destroy(c->args);
    typecmp_ctx_deinit();
    rf_stringx_deinit(&c->err_buff);
    rf_deinit();
}

void compiler_destroy(struct compiler *c)
{
    compiler_deinit(c);
    free(c);
    g_compiler_instance = NULL;
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

    return compiler_new_front(c, &c->args->input, true);
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

static bool compiler_visit_unmarked_module(struct module *m, unsigned i, bool *marks)
{
    // not a DAG, cyclic dependency detected
    if (marks[i]) {
        return false;
    }

    marks[i] = true;
    struct module **dependency;
    darray_foreach(dependency, m->dependencies) {

        // find index of dependency in marks
        unsigned found_index = compiler_get_module_index(*dependency);
        // visit the module
        if (!compiler_visit_unmarked_module(*dependency, found_index, marks)) {
            return false;
        }
    }

    // add module 'm to the sorted list
    rf_ilist_add(&compiler_instance_get()->sorted_modules, &m->ln);
    return true;
}

static bool compiler_resolve_dependencies()
{
    struct compiler *c = g_compiler_instance;
    struct module **mod;
    rf_ilist_head_init(&c->sorted_modules);

    // topologically sort the dependency DAG
    unsigned int i;
    bool *marks;
    RF_CALLOC(marks, darray_size(c->modules), sizeof(bool), return false);

    while (true) {
        i = 0;
        darray_foreach(mod, c->modules) {
            // if module is unmarked
            if (!marks[i]) {
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
    // make sure all files are parsed
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (!front_ctx_parse(front)) {
            return false;
        }
    }

    // determine the dependencies of all the modules
    struct module **mod;
    darray_foreach(mod, c->modules) {
        if (!analyzer_determine_dependencies(*mod, (*mod)->front->parser)) {
            return false;
        }
    }

    // resolve dependencies and figure out analysis order
    return compiler_resolve_dependencies();
}

bool compiler_process()
{
    struct compiler *c = g_compiler_instance;
    
    // add the standard library to the front contexts
    struct front_ctx *stdlib_front;
    const struct RFstring stdlib = RF_STRING_STATIC_INIT(RF_LANG_CORE_ROOT"/stdlib/io.rf");
    if (!(stdlib_front = compiler_new_front(c, &stdlib, false))) {
        RF_ERROR("Failed to add standard library to the front_ctxs");
        return false;
    }

    if (!compiler_preprocess_fronts()) {
        return false;
    }
    
    // now analyze the modules in the topologically sorted order
    struct module *mod;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        if (!module_analyze(mod)) {
            RF_ERROR("Failed to analyze a module");
            return false;
        }
    }

#if 0 // don't call serializer at all for now
    enum serializer_rc rc = serializer_process(c->serializer, front);
    if (rc == SERC_SUCCESS_EXIT || rc == SERC_SUCCESS_EXIT) {
        return rc;
    }
#endif

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

struct RFstringx *compiler_get_errors(struct compiler *c)
{
    struct front_ctx *front;
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (!info_ctx_get_messages_fmt(front->info, MESSAGE_ANY, &c->err_buff)) {
        return NULL;
        }
    }

    return &c->err_buff;
}
