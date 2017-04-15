#include "testsupport_analyzer.h"
#include "../testsupport_front.h"

#include <info/info.h>
#include <info/msg.h>
#include <rfbase/refu.h>
#include <check.h>

#include <ast/ast.h>

#include <types/type_function.h>
#include <types/type_arr.h>

#define ck_analyzer_check_abort(file_, line_, msg_, ...)                \
    ck_abort_msg("Checking expected parser error from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

static struct analyzer_testdriver i_analyzer_test_driver_;
bool analyzer_testdriver_init(struct analyzer_testdriver *d)
{
    darray_init(d->types);
    return true;
}

void analyzer_testdriver_deinit(struct analyzer_testdriver *d)
{
    struct type **t;
    darray_foreach(t, d->types) {
        type_free(*t, front_testdriver_module()->types_pool);
    }
    darray_free(d->types);
}

struct analyzer_testdriver *get_analyzer_testdriver()
{
    return &i_analyzer_test_driver_;
}


void setup_analyzer_tests()
{
    setup_front_tests();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
}

void setup_analyzer_tests_before_firstpass()
{
    setup_front_tests();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
    type_creation_ctx_init();
}

void setup_analyzer_tests_before_firstpass_with_filelog()
{
    setup_front_tests_with_file_log();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
    type_creation_ctx_init();
}

void setup_analyzer_tests_no_stdlib()
{
    setup_front_tests_no_stdlib();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
}

void setup_analyzer_tests_no_source()
{
    setup_front_tests();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
    // empty source file
    front_testdriver_new_ast_main_source(rf_string_empty_get());
    // and since it's empty get to the analysis stage (some tests need this)
    testsupport_analyzer_prepare();
}

void setup_analyzer_tests_with_filelog()
{
    setup_front_tests_with_file_log();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
}

void teardown_analyzer_tests()
{
    analyzer_testdriver_deinit(&i_analyzer_test_driver_);
    teardown_front_tests();
}

void teardown_analyzer_tests_before_firstpass()
{
    type_creation_ctx_deinit();
    analyzer_testdriver_deinit(&i_analyzer_test_driver_);
    teardown_front_tests();
}


struct type *testsupport_analyzer_type_create_simple_elementary(enum elementary_type etype)
{
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(front_testdriver_module());
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_ELEMENTARY;
    t->elementary.etype = etype;

    darray_append(adriver->types, t);
    return t;
}

struct type *i_testsupport_analyzer_type_create_operator(
    enum typeop_type type,
    unsigned int argsn,
    ...)
{
    va_list valist;
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(front_testdriver_module());
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = type;
    darray_init(t->operator.operands);

    va_start(valist, argsn);
    unsigned int i;
    for (i = 0; i < argsn; ++i) {
        darray_append(t->operator.operands, va_arg(valist, struct type*));
    }
    va_end(valist);

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_defined(
    const struct RFstring *name,
    struct type *type)
{
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(front_testdriver_module());
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_DEFINED;
    t->defined.name = name;
    t->defined.type = type;

    darray_append(adriver->types, t);
    return t;
}

struct type *i_testsupport_analyzer_type_create_array(
    const struct type *member_type,
    int64_t *arr,
    size_t arr_size)
{
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(front_testdriver_module());
    ck_assert_msg(t, "Failed to allocate type");

    struct arr_int64 dimensions;
    testsupport_arr_with_size_to_darray(dimensions, arr, arr_size, int64_t);
    type_array_init(t, member_type, &dimensions);

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_function(struct type *arg,
                                                       struct type *ret)
{
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(front_testdriver_module());
    ck_assert_msg(t, "Failed to allocate type");
    type_function_init(t, arg, ret);
    darray_append(adriver->types, t);
    return t;
}

bool ck_assert_analyzer_errors_impl(struct info_msg *exp_errors,
                                    unsigned num,
                                    const char *filename,
                                    unsigned int line)
{
    struct info_msg *msg;
    struct info_ctx_msg_iterator iter;
    struct compiler *c = compiler_instance_get();
    unsigned i = 0;

    // check against errors from all front_ctxs of the compiler
    struct front_ctx *front;
    rf_ilist_for_each(&c->front_ctxs, front, ln) {

        info_ctx_get_iter(front->info, MESSAGE_ANY, &iter);

        while ((msg = info_ctx_msg_iterator_next(&iter))) {
            if (i >= num) {
                ck_analyzer_check_abort(
                    filename, line,
                    "Got more analyzer errors than the expected %u. The extra "
                    "error we got is:\n\""RFS_PF"\".",
                    num,
                    RFS_PA(&msg->s)
                );
                return false;
            }

            // check for error message string
            if (!rf_string_equal(&msg->s, &exp_errors[i].s)) {
                ck_analyzer_check_abort(
                    filename, line,
                    "For analyzer error number %u: Got:\n\""RFS_PF"\"\n"
                    "but expected:\n\""RFS_PF"\"", i,
                    RFS_PA(&msg->s),
                    RFS_PA(&exp_errors[i].s)
                );
                return false;
            }

            // check for error message location
            if (!inplocation_mark_equal(&msg->start_mark, &exp_errors[i].start_mark)) {
                ck_analyzer_check_abort(
                    filename, line,
                    "For analyzer error number %u got different start location marks. Got:\n"
                    INPLOCMARKS_FMT " but"
                    " expected:\n"INPLOCMARKS_FMT,
                    i,
                    INPLOCMARKS_ARG(front->info->file, &msg->start_mark, &msg->end_mark),
                    INPLOCMARKS_ARG(front->info->file, &exp_errors[i].start_mark,
                                    &exp_errors[i].end_mark));
                return false;
            }

            if (info_msg_has_end_mark(msg) &&
                !inplocation_mark_equal(&msg->end_mark, &exp_errors[i].end_mark)) {
                ck_analyzer_check_abort(
                    filename, line,
                    "For analyzer error number %u got different end location marks. Got:\n"
                    INPLOCMARKS_FMT " but"
                    " expected:\n"INPLOCMARKS_FMT,
                    i,
                    INPLOCMARKS_ARG(front->info->file, &msg->start_mark, &msg->end_mark),
                    INPLOCMARKS_ARG(front->info->file, &exp_errors[i].start_mark,
                                    &exp_errors[i].end_mark));
                return false;
            }

            // check for message type
            if (msg->type != exp_errors[i].type) {
                ck_analyzer_check_abort(
                    filename, line,
                    "For analyzer error number %u got different message types."
                    " Got:\n\""RFS_PF"\" but expected: \"" RFS_PF"\"",
                    i,
                    RFS_PA(info_msg_type_to_str(msg->type)),
                    RFS_PA(info_msg_type_to_str(exp_errors[i].type))
                );
                return false;
            }


            i ++;
        }
    }

    if (i != num) {
        ck_analyzer_check_abort(
            filename, line,
            "Expected %u analyzer messages but found %u", num, i);
        return false;
    }

    return true;
}

void i_ck_assert_modules_order(const struct RFstring *expected_module_names,
                               unsigned int expected_num,
                               const char *filename,
                               unsigned int line)
{
    unsigned int i = 0;
    struct compiler *c = compiler_instance_get();
    // check the modules in the topologically sorted order
    struct module *mod;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        ck_assert_msg(rf_string_equal(&expected_module_names[i], module_name(mod)),
                      "Dependency error. Expected module \""RFS_PF"\" at "
                      "position %u but found module \""RFS_PF"\". At %s:%uu",
                      RFS_PA(&expected_module_names[i]),
                      i,
                      RFS_PA(module_name(mod)),
                      filename, line);
        ++i;
    }

    ck_assert_msg(i == expected_num,
                  "Mismatch of expected modules at dependency check. Expected "
                  "%u modules but found %i modules. At %s:%u",
                  expected_num, i, filename, line);
}
