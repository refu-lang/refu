#include "testsupport_analyzer.h"
#include "../testsupport_front.h"

#include <info/info.h>
#include <info/msg.h>
#include <refu.h>
#include <check.h>

#include <ast/ast.h>

#include <types/type_function.h>

#define ck_analyzer_check_abort(file_, line_, msg_, ...)                 \
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
    struct front_testdriver *fdriver = get_front_testdriver();
    darray_foreach(t, d->types) {
        type_free(*t, fdriver->front.analyzer);
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


struct type *testsupport_analyzer_type_create_elementary(enum elementary_type etype,
                                                         bool constant)
{
    struct front_testdriver *fdriver = get_front_testdriver();
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(fdriver->front.analyzer);
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_ELEMENTARY;
    t->elementary.etype = etype;
    t->elementary.is_constant = constant;

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_operator(enum typeop_type type,
                                                       struct type *left,
                                                       struct type *right)
{
    struct front_testdriver *fdriver = get_front_testdriver();
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(fdriver->front.analyzer);
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = type;
    t->operator.left = left;
    t->operator.right = right;

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_defined(const struct RFstring *name,
                                                      struct type *type)
{
    struct front_testdriver *fdriver = get_front_testdriver();
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(fdriver->front.analyzer);
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_DEFINED;
    t->defined.name = name;
    t->defined.type = type;

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_leaf(const struct RFstring *id,
                                                   struct type *type)
{
    struct front_testdriver *fdriver = get_front_testdriver();
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(fdriver->front.analyzer);
    ck_assert_msg(t, "Failed to allocate type");

    t->category = TYPE_CATEGORY_LEAF;
    t->leaf.id = id;
    t->leaf.type = type;

    darray_append(adriver->types, t);
    return t;
}

struct type *testsupport_analyzer_type_create_function(struct type *arg,
                                                       struct type *ret)
{
    struct front_testdriver *fdriver = get_front_testdriver();
    struct analyzer_testdriver *adriver = get_analyzer_testdriver();
    struct type *t;
    t = type_alloc(fdriver->front.analyzer);
    ck_assert_msg(t, "Failed to allocate type");
    type_function_init(t, arg, ret);
    darray_append(adriver->types, t);
    return t;
}

bool ck_assert_analyzer_errors_impl(struct info_ctx *info,
                                    struct info_msg *exp_errors,
                                    unsigned num,
                                    const char *filename,
                                    unsigned int line)
{
    struct info_msg *msg;
    struct info_ctx_msg_iterator iter;
    unsigned i = 0;
    info_ctx_get_iter(info, MESSAGE_ANY, &iter);

    while ((msg = info_ctx_msg_iterator_next(&iter))) {

        // check for error message string
        if (!rf_string_equal(&msg->s, &exp_errors[i].s)) {
            ck_analyzer_check_abort(
                filename, line,
                "For analyzer error number %u: Got:\n\""RF_STR_PF_FMT"\"\n"
                "but expected:\n\""RF_STR_PF_FMT"\"", i,
                RF_STR_PF_ARG(&msg->s), RF_STR_PF_ARG(&exp_errors[i].s));
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
                INPLOCMARKS_ARG(info->file, &msg->start_mark, &msg->end_mark),
                INPLOCMARKS_ARG(info->file, &exp_errors[i].start_mark,
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
                INPLOCMARKS_ARG(info->file, &msg->start_mark, &msg->end_mark),
                INPLOCMARKS_ARG(info->file, &exp_errors[i].start_mark,
                                &exp_errors[i].end_mark));
            return false;
        }

        // check for message type
        if (msg->type != exp_errors[i].type) {
            ck_analyzer_check_abort(
                filename, line,
                "For analyzer error number %u got different message types. Got:\n"
                "\""RF_STR_PF_FMT"\" but expected: \"" RF_STR_PF_FMT"\"",
                i,
                RF_STR_PF_ARG(info_msg_type_to_str(msg->type)),
                RF_STR_PF_ARG(info_msg_type_to_str(exp_errors[i].type)));
            return false;
        }


        i ++;
    }

    if (i != num) {
            ck_analyzer_check_abort(
                filename, line,
                "Expected %u analyzer messages but found %u", num, i);
            return false;
    }

    return true;
}
