#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <info/msg.h>
#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include <ast/ast.h>
#include <ast/block.h>
#include <ast/identifier.h>
#include <ast/function.h>
#include <ast/vardecl.h>
#include <ast/type.h>

#include <types/type.h>

#include "../../src/analyzer/analyzer_pass1.h"

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

/* -- simple symbol table functionality tests -- */

START_TEST(test_typecheck_assignment_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "name:string\n"
        "b:f64\n"
        "a = 456\n"
        "name = \"Lefteris\"\n"
        "b = 0.231\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST


START_TEST(test_typecheck_assignment_literal_to_var) {
    //TODO: This may be showing a warning to me and not seeing it right now
    // The warning should not be there. Even if all constants are u64 they could
    // be assigned to a smaller variable if possible like here.
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u8\n"
        "a = 103\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
        struct info_msg messages[] = {
        /* TESTSUPPORT_INFOMSG_INIT_BOTH( */
        /*     &d->front.file, */
        /*     MESSAGE_SEMANTIC_WARNING, */
        /*     "Assignment from a larger to a smaller builtin type." */
        /*     "\"u64\" to \"u8\"", */
        /*     2, 0, 2, 6) */
    };

    testsupport_typecheck_prepare(d);
    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_with_messages(d, true, messages);
    /* ck_assert_typecheck_ok(d); */
} END_TEST

START_TEST(test_typecheck_assignment_invalid_storage_size) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u8\n"
        "a = 456\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. "
            "Can't assign \"u16\" to \"u8\"",
            1, 0, 1, 6)
    };

    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, false);

    ck_assert_typecheck_with_messages(d, false, messages);
} END_TEST

START_TEST(test_typecheck_assignment_invalid1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "name:string\n"
        "a = 456\n"
        "name = \"Celina\"\n"
        "a = name\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"string\" to \"u64\"",
            4, 0, 4, 7)
    };

    ck_assert_typecheck_with_messages(d, false, messages);
} END_TEST

START_TEST(test_typecheck_addition_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "name:string\n"
        "a = b + c\n"
        "name = \"foo\" + \"bar\""
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST
START_TEST(test_typecheck_addition_complex) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "a = b + c + 321 + 234\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *st1 = tcase_create("analyzer_typecheck_assignments");
    tcase_add_checked_fixture(st1,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st1, test_typecheck_assignment_simple);
    tcase_add_test(st1, test_typecheck_assignment_literal_to_var);
    tcase_add_test(st1, test_typecheck_assignment_invalid_storage_size);
    tcase_add_test(st1, test_typecheck_assignment_invalid1);

    TCase *st2 = tcase_create("analyzer_typecheck_additions");
    tcase_add_checked_fixture(st2,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st2, test_typecheck_addition_simple);
    tcase_add_test(st2, test_typecheck_addition_complex);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}


