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

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

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

    ck_assert_typecheck_ok(d, true);
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
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. "
            "Can't assign \"u16\" to \"u8\"",
            1, 0, 1, 6)
    };

    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, false);

    ck_assert_typecheck_with_messages(d, false, messages, true);
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
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"string\" to \"u64\"",
            4, 0, 4, 7)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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

    ck_assert_typecheck_ok(d, true);
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

    ck_assert_typecheck_ok(d, true);
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

    ck_assert_typecheck_ok(d, true);
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

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_member_access) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person {name:string, age:u32}\n"
        "{\n"
        "p:person\n"
        "a:u32 = 42\n"
        "b:u32 = a + p.age\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_invalid_member_access) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person {name:string, age:u32}\n"
        "{\n"
        "p:person\n"
        "a:u32 = 42\n"
        "b:u32 = a + p.craze\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Could not find member \"craze\" in type \"person\"",
            4, 12, 4, 18),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"+\" can not be determined",
            4, 12, 4, 18),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            4, 8, 4, 18),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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

    ck_assert_typecheck_ok(d, true);
} END_TEST

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

START_TEST(test_typecheck_valid_function_call_print) {
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
            "\"string,u16\"",
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
            "\"f32\" to \"u64\"",
            6, 0, 6, 32),
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
            "\"string,u32\" which does not match the expected type of \"nil\"",
            6, 8, 6, 32),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign \"nil\" to \"u64\"",
            6, 0, 6, 32)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST(test_typecheck_valid_function_impl) {
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

START_TEST(test_typecheck_valid_custom_type_and_fncall1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something(a:person) -> string\n"
        "{\n"
        "return \"something\""
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_custom_type_and_fncall2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something(a:person, b:u64) -> string\n"
        "{\n"
        "return \"something\" + a.name"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_valid_custom_type_constructor) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something() -> string\n"
        "{\n"
        "a:person = person(\"Celina\", 18)\n"
        "return a.name"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_invalid_custom_type_constructor) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }\n"
        "fn do_something() -> string\n"
        "{\n"
        "a:person = person(\"Celina\")\n"
        "return a.name"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "constructor person() is called with argument type of "
            "\"string\" which does not match the expected type of \"string,u32\"",
            3, 11, 3, 26),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *t_assign_val = tcase_create("analyzer_typecheck_assignments");
    tcase_add_checked_fixture(t_assign_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_assign_val, test_typecheck_assignment_simple);

    TCase *t_assign_inv = tcase_create("analyzer_typecheck_invalid_assignments");
    tcase_add_checked_fixture(t_assign_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_assign_inv, test_typecheck_assignment_invalid_storage_size);
    tcase_add_test(t_assign_inv, test_typecheck_assignment_invalid_string_to_int);

    TCase *t_bop_val = tcase_create("analyzer_typecheck_binary_operations");
    tcase_add_checked_fixture(t_bop_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_bop_val, test_typecheck_valid_addition_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_subtraction_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_multiplication_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_division_simple);

    TCase *t_access_val = tcase_create("analyzer_typecheck_valid_access_operations");
    tcase_add_checked_fixture(t_access_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_access_val, test_typecheck_valid_member_access);

    TCase *t_access_inv = tcase_create("analyzer_typecheck_invalid_access_operations");
    tcase_add_checked_fixture(t_access_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_access_inv, test_typecheck_invalid_member_access);

    TCase *t_vardecl_val = tcase_create("analyzer_typecheck_misc");
    tcase_add_checked_fixture(t_vardecl_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_vardecl_val, test_typecheck_variable_declarations);
    // TODO: Test where there are errors in two different parts of the code
    //       to assert the continuation of the traversal works

    TCase *t_func_val = tcase_create("analyzer_typecheck_functions");
    tcase_add_checked_fixture(t_func_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call0);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call1);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call2);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call_print);
    tcase_add_test(t_func_val, test_typecheck_valid_function_impl);


    TCase *t_func_inv = tcase_create("analyzer_typecheck_invalid_functions");
    tcase_add_checked_fixture(t_func_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_arguments);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_return);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_with_nil_arg_and_ret);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_impl_return);

    TCase *t_custom_types_val = tcase_create("analyzer_typecheck_valid_custom_types");
    tcase_add_checked_fixture(t_custom_types_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall1);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall2);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_constructor);

    TCase *t_custom_types_inv = tcase_create("analyzer_typecheck_invalid_custom_types");
    tcase_add_checked_fixture(t_custom_types_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_inv, test_typecheck_invalid_custom_type_constructor);

    suite_add_tcase(s, t_assign_val);
    suite_add_tcase(s, t_assign_inv);
    suite_add_tcase(s, t_bop_val);
    suite_add_tcase(s, t_access_val);
    suite_add_tcase(s, t_access_inv);
    suite_add_tcase(s, t_vardecl_val);
    suite_add_tcase(s, t_func_val);
    suite_add_tcase(s, t_func_inv);
    suite_add_tcase(s, t_custom_types_val);
    suite_add_tcase(s, t_custom_types_inv);
    return s;
}


