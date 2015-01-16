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

START_TEST(test_typecheck_assignment_invalid_string_to_int) {
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

START_TEST(test_typecheck_valid_addition_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "name:string\n"
        "a = b + c + 321 + 234\n"
        "name = \"foo\" + \"bar\""
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_valid_subtraction_simple) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "foo:string\n"
        "foo = \"FooBar\" - \"Bar\"\n"
        "a = b - c - 321 - 234\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_valid_multiplication_simple) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "a = b * c * 234\n"
        "d:f64\n"
        "d = 3.14 * 0.14\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_valid_division_simple) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "a = b / c\n"
        "d:f64\n"
        "d = 3.14 / 0.14\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_variable_declarations) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u64 = 523432\n"
        "b:u64 = 123 + b\n"
        "s:string = \"Foo\" + \"Bar\"\n"
        "d:f32 = 98 / 3.21\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_valid_function_call) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16) -> f32\n"
        "{\n"
        "return age * 0.14\n"
        "}\n"
        "{\n"
        "name:string = \"Francis\"\n"
        "do_something(name, 45)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_arguments) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16) -> f32\n"
        "{\n"
        "return age * 0.14\n"
        "}\n"
        "{\n"
        "a:u32 = 15\n"
        "do_something(a, \"Berlin\")\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, false);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Argument 1 of do_something() function call does not match the "
            "function signature. Expected \"string\" but got \"u32\".",
            6, 0, 6, 24),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Argument 2 of do_something() function call does not match the "
            "function signature. Expected \"u16\" but got \"string\".",
            6, 0, 6, 24)
    };

    ck_assert_typecheck_with_messages(d, false, messages);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_number_of_arguments) {
#if 0 //work in progress
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16) -> f32\n"
        "{\n"
        "return age * 0.14\n"
        "}\n"
        "fn do_something_else(name:string, age:u16, alien:bool) -> f32\n"
        "{\n"
        "return age * 0.14\n"
        "}\n"
        "{\n"
        "a:u32 = 15\n"
        "do_something(\"Berlin\")\n"
        "do_something_else(\"Berlin\", a)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, false);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Invalid number of arguments provided to function \"do_something()\"."
            " Expected 2 arguments but provided 1."
            10, 0, 10, 21),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Invalid number of arguments provided to function \"do_something_else()\"."
            " Expected 3 arguments but provided 2."
            11, 0, 11, 29),
    };

    ck_assert_typecheck_with_messages(d, false, messages);
#endif
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *st1 = tcase_create("analyzer_typecheck_assignments");
    tcase_add_checked_fixture(st1,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st1, test_typecheck_assignment_simple);
    tcase_add_test(st1, test_typecheck_assignment_invalid_storage_size);
    tcase_add_test(st1, test_typecheck_assignment_invalid_string_to_int);

    TCase *st2 = tcase_create("analyzer_typecheck_binary_operations");
    tcase_add_checked_fixture(st2,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st2, test_typecheck_valid_addition_simple);
    tcase_add_test(st2, test_typecheck_valid_subtraction_simple);
    tcase_add_test(st2, test_typecheck_valid_multiplication_simple);
    tcase_add_test(st2, test_typecheck_valid_division_simple);

    TCase *st3 = tcase_create("analyzer_typecheck_misc");
    tcase_add_checked_fixture(st3,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st3, test_typecheck_variable_declarations);

    TCase *st4 = tcase_create("analyzer_typecheck_functions");
    tcase_add_checked_fixture(st4,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st4, test_typecheck_valid_function_call);
    tcase_add_test(st4, test_typecheck_invalid_function_call_arguments);
    tcase_add_test(st4, test_typecheck_invalid_function_call_number_of_arguments);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    suite_add_tcase(s, st3);
    suite_add_tcase(s, st4);
    return s;
}


