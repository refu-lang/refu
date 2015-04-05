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

START_TEST(test_typecheck_negative_int_variable_declarations) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32 = -23432\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_complex_type_in_variable_declaration) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:(b:int | c:string)\n"
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
            "\"string\" which does not match the expected type of \"string,u32\".",
            3, 11, 3, 26),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            3, 11, 3, 26),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_valid_assignment_from_block1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        \"a_string_literal\"\n"
        "    }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_assignment_from_block2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:u64, b:string }\n"
        "{\n"
        "    a:foo = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        c:foo = foo(565, \"Berlin\")\n"
        "    }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_invalid_assignment_from_block1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        \"a_string_literal\"\n"
        "        b:u32 = 13 + 25\n"
        "    }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"u32\" to \"string\". Unable to convert from \"u32\" to \"string\".",
            1, 4, 5, 4),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_invalid_assignment_from_block2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:u64, b:string }\n"
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        c:foo = foo(565, \"Berlin\")\n"
        "    }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"foo\" to \"string\".",
            2, 4, 6, 4),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_valid_if_stmt) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:u64 = 1453\n"
        "    b:u64"
        "    if a == 1453 { b = 1 } else { b = 0 }\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);
    ck_assert_typecheck_ok(d, true);
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *t_typecheck_misc = tcase_create("typecheck_misc");
    tcase_add_checked_fixture(t_typecheck_misc,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_typecheck_misc, test_typecheck_variable_declarations);
    tcase_add_test(t_typecheck_misc, test_typecheck_negative_int_variable_declarations);
    tcase_add_test(t_typecheck_misc, test_typecheck_complex_type_in_variable_declaration);
    // TODO: Test where there are errors in two different parts of the code
    //       to assert the continuation of the traversal works

    TCase *t_func_val = tcase_create("typecheck_functions");
    tcase_add_checked_fixture(t_func_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call0);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call1);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call2);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call_print_string);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call_print_int);
    tcase_add_test(t_func_val, test_typecheck_valid_function_call_with_sum_args);
    tcase_add_test(t_func_val, test_typecheck_valid_function_impl);


    TCase *t_func_inv = tcase_create("typecheck_invalid_functions");
    tcase_add_checked_fixture(t_func_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_arguments);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_return);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_return2);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_with_nil_arg_and_ret);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_undeclared_identifier);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_call_with_sum_args);
    tcase_add_test(t_func_inv, test_typecheck_invalid_function_impl_return);

    TCase *t_custom_types_val = tcase_create("typecheck_valid_custom_types");
    tcase_add_checked_fixture(t_custom_types_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall1);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall2);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_constructor);

    TCase *t_custom_types_inv = tcase_create("typecheck_invalid_custom_types");
    tcase_add_checked_fixture(t_custom_types_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_inv, test_typecheck_invalid_custom_type_constructor);

    TCase *t_block_val = tcase_create("typecheck_valid_blocks");
    tcase_add_checked_fixture(t_block_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_block_val, test_typecheck_valid_assignment_from_block1);
    tcase_add_test(t_block_val, test_typecheck_valid_assignment_from_block2);


    TCase *t_block_inv = tcase_create("typecheck_invalid_blocks");
    tcase_add_checked_fixture(t_block_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_block_inv, test_typecheck_invalid_assignment_from_block1);
    tcase_add_test(t_block_inv, test_typecheck_invalid_assignment_from_block2);

    TCase *t_if_val = tcase_create("typecheck_valid_if");
    tcase_add_checked_fixture(t_if_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_if_val, test_typecheck_valid_if_stmt);

    suite_add_tcase(s, t_typecheck_misc);
    suite_add_tcase(s, t_func_val);
    suite_add_tcase(s, t_func_inv);
    suite_add_tcase(s, t_custom_types_val);
    suite_add_tcase(s, t_custom_types_inv);
    suite_add_tcase(s, t_block_val);
    suite_add_tcase(s, t_block_inv);
    suite_add_tcase(s, t_if_val);
    return s;
}


