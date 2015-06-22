#include "compiler.h"

#include <refu.h>
#include <Utils/memory.h>

#include <info/info.h>
#include <types/type_comparisons.h>
#include <compiler_args.h>
#include <ast/ast.h>
#include <front_ctx.h>
#include <serializer/serializer.h>
#include <backend/llvm.h>

struct rir_module;

bool compiler_init(struct compiler *c)
{
    RF_STRUCT_ZERO(c);

    // initialize Refu library
    rf_init(LOG_TARGET_STDOUT,
            NULL,
            LOG_WARNING,
            RF_DEFAULT_TS_MBUFF_INITIAL_SIZE,
            RF_DEFAULT_TS_SBUFF_INITIAL_SIZE
    );

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

void compiler_deinit(struct compiler *c)
{
    struct front_ctx *front;
    struct front_ctx *tmp;
    rf_ilist_for_each_safe(&c->front_ctxs, front, tmp, ln) {
        front_ctx_destroy(front);
    }

    serializer_destroy(c->serializer);
    compiler_args_destroy(c->args);
    typecmp_ctx_deinit();
    rf_stringx_deinit(&c->err_buff);
    rf_deinit();
}

static struct front_ctx *compiler_new_front(struct compiler *c,
                                            const struct RFstring *input_name,
                                            bool add_at_start)
{
    struct front_ctx *front;
    if (!(front = front_ctx_create(c->args, input_name))) {
        RF_ERROR("Failure at frontend context initialization");
        return NULL;
    }
    if (add_at_start) {
        rf_ilist_add(&c->front_ctxs, &front->ln);
    } else {
        rf_ilist_add_tail(&c->front_ctxs, &front->ln);
    }
    return front;
}

bool compiler_pass_args(struct compiler *c, int argc, char **argv)
{
    if (!compiler_args_parse(c->args, argc, argv)) {
        return false;
    }

    // do not proceed any further if we got request for help
    if (compiler_args_help_is_requested(c->args)) {
        return true;
    }

    return compiler_new_front(c, &c->args->input, true);
}

bool compiler_init_with_args(struct compiler *c, int argc, char **argv)
{
    if (!compiler_init(c)) {
        return false;
    }

    return compiler_pass_args(c, argc, argv);
}

static bool compiler_preprocess_fronts(struct compiler *c)
{
    struct front_ctx *front;
    // make sure all files are parsed
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (!front_ctx_parse(front)) {
            return false;
        }
    }

    // determine the dependencies of all the modules
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (!analyzer_determine_dependencies(front->analyzer, front->parser)) {
            return false;
        }
    }

    //TODO: resolve dependencies and figure out analyze order
    
    return true;
}

static bool compiler_process_front(struct compiler *c,
                                   struct front_ctx *front,
                                   struct front_ctx *stdlib)
{
    // TODO: now here you will know the order of analysis so analyze the modules in order
    struct analyzer *analyzer = front_ctx_process(front, stdlib);
    if (!analyzer) {
        RF_ERROR("Failure to parse and analyze the input");

        // for now temporarily just dump all messages in the info context
        // TODO: fix
        if (!info_ctx_get_messages_fmt(front->info, MESSAGE_ANY, &c->err_buff)) {
            RF_ERROR("Could not retrieve messages from the info context");
            return false;
        }
        printf(RF_STR_PF_FMT, RF_STR_PF_ARG(&c->err_buff));
        return false;
    }

    // for now at least, if an empty file is given quit here with a message
    if (ast_node_get_children_number(analyzer->root) == 0) {
        ERROR("Provided an empty file for compilation");
        return false;
    }

    return true;
}

bool compiler_process(struct compiler *c)
{
    // add the standard library to the front contexts
    struct front_ctx *stdlib_front;
    const struct RFstring stdlib = RF_STRING_STATIC_INIT(RF_LANG_CORE_ROOT"/stdlib/io.rf");
    if (!(stdlib_front = compiler_new_front(c, &stdlib, true))) {
        RF_ERROR("Failed to add standard library to the front_ctxs");
        return false;
    }

    // first and foremost process the standard library front context
    if (!compiler_process_front(c, stdlib_front, NULL)) {
        RF_ERROR("Failed to analyze stdlib module");
        return false;
    }

    struct front_ctx *front;
    rf_ilist_for_each(&c->front_ctxs, front, ln) {
        if (front == stdlib_front) {
            continue;
        }
        if (!compiler_process_front(c, front, stdlib_front)) {
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

    if (!bllvm_generate(&c->front_ctxs, c->args)) {
        RF_ERROR("Failed to create the LLVM IR from the Refu IR");
        return false;
    }

    return true;
}

bool compiler_help_requested(struct compiler *c)
{
    return compiler_args_check_and_display_help(c->args);
}
