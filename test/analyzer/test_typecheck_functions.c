#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_typecheck_valid_function_call0) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something() -> u32\n"
        "{\n"
        "return 42\n"
        "}\n"
        "{\n"
        "a:u64 = do_something()\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_function_call1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(age:u16) -> f32\n"
        "{\n"
        "return age + 0.14\n"
        "}\n"
        "{\n"
        "do_something(13)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_function_call2) {
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

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_function_call_print_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "name:string = \"Francis\"\n"
        "print(name)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

#if 0 // temporary make print accept only string for debugging purposes. TODO: fix
START_TEST(test_typecheck_valid_function_call_print_int) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u32 = 64\n"
        "print(a)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST
#endif

START_TEST(test_typecheck_valid_function_call_with_sum_args) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:i64 | b:string) { }\n"
        "{\n"
        "foo(45)\n"
        "foo(\"s\")\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
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
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "function do_something() is called with argument type of "
            "\"u32,string\" which does not match the expected type of "
            "\"string,u16\". Unable to convert from \"u32\" to \"string\".",
            6, 0, 6, 24),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_return) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16) -> f32\n"
        "{\n"
        "return age * 0.14\n"
        "}\n"
        "{\n"
        "a:u32 = 15\n"
        "c:u64 = do_something(\"Berlin\", a)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"f32\" to \"u64\". Unable to convert from \"f32\" to \"u64\".",
            6, 0, 6, 32),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_return2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16) -> f32\n"
        "{\n"
        "return s\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Undeclared identifier \"s\"",
            2, 7, 2, 7),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_with_nil_arg_and_ret) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something()\n"
        "{\n"
        "11\n"
        "}\n"
        "{\n"
        "a:u32 = 15\n"
        "c:u64 = do_something(\"Berlin\", a)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "function do_something() is called with argument type of "
            "\"string,u32\" which does not match the expected type of \"nil\".",
            6, 8, 6, 32),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            6, 8, 6, 32)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_invalid_function_call_undeclared_identifier) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn main()->u32{\n"
        "    a:string = \"a\"\n"
        "    print(b)\n"
        "    return 1\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Undeclared identifier \"b\"",
            2, 10, 2, 10),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST(test_typecheck_invalid_function_call_with_sum_args) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:u64 | b:string) { }\n"
        "{\n"
        "foo(45)\n"
        "foo(true)\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "function foo() is called with argument type of \"bool\" which does "
            "not match the expected type of \"u64|string\". Unable to convert from "
            "\"bool\" to \"string\". An implicit conversion already happened.",
            3, 0, 3, 8),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_valid_function_impl) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something(name:string, age:u16, height:u16, weight:u16, vegetarian:bool) -> u32\n"
        "{\n"
        "    if vegetarian {\n"
        "        return age + height * weight\n"
        "    } else {\n"
        "        return weight * age - height\n"
        "    }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_function_impl_matchexp_body_void_return) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn action(a:i32 | b:string)\n"
        "a:i32    => print(\"int\")\n"
        "b:string => print(b)\n"
        "\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_function_impl_matchexp_body_with_return) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn action(a:i32 | b:string) -> i32\n"
        "a:i32    => a\n"
        "b:string => 0\n"
        "\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_function_impl_matchexp_body_with_complicated_return) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn action(a:i32 | b:string) -> i32 | string\n"
        "a:i32    => a\n"
        "b:string => b\n"
        "\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_invalid_function_impl_return) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_something() -> string\n"
        "{\n"
        "return 15"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Return statement type \"u8\" does not match the "
            "expected return type of \"string\"",
            2, 0, 2, 8),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST



Suite *analyzer_typecheck_functions_suite_create(void)
{
    Suite *s = suite_create("typecheck_functions");

    TCase *t_call_val = tcase_create("typecheck_valid_function_calls");
    tcase_add_checked_fixture(t_call_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_call_val, test_typecheck_valid_function_call0);
    tcase_add_test(t_call_val, test_typecheck_valid_function_call1);
    tcase_add_test(t_call_val, test_typecheck_valid_function_call2);
    tcase_add_test(t_call_val, test_typecheck_valid_function_call_print_string);
    #if 0 // temporary make print accept only string for debugging purposes. TODO: fix
    tcase_add_test(t_call_val, test_typecheck_valid_function_call_print_int);
    #endif
    tcase_add_test(t_call_val, test_typecheck_valid_function_call_with_sum_args);

    TCase *t_call_inv = tcase_create("typecheck_invalid_function_calls");
    tcase_add_checked_fixture(t_call_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_arguments);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_return);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_return2);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_with_nil_arg_and_ret);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_undeclared_identifier);
    tcase_add_test(t_call_inv, test_typecheck_invalid_function_call_with_sum_args);

    TCase *t_impl_val = tcase_create("typecheck_valid_function_implementations");
    tcase_add_checked_fixture(t_impl_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_impl_val, test_typecheck_valid_function_impl);
    tcase_add_test(t_impl_val, test_typecheck_valid_function_impl_matchexp_body_void_return);
    tcase_add_test(t_impl_val, test_typecheck_valid_function_impl_matchexp_body_with_return);
    tcase_add_test(t_impl_val, test_typecheck_valid_function_impl_matchexp_body_with_complicated_return);

    TCase *t_impl_inv = tcase_create("typecheck_invalid_function_implementations");
    tcase_add_checked_fixture(t_impl_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_impl_inv, test_typecheck_invalid_function_impl_return);


    suite_add_tcase(s, t_call_val);
    suite_add_tcase(s, t_call_inv);
    suite_add_tcase(s, t_impl_val);
    suite_add_tcase(s, t_impl_inv);

    return s;
}


