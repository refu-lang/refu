#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rfbase/string/core.h>
#include <rfbase/utils/hash.h>

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

START_TEST (test_typecheck_implicit_conversion_bool_to_int) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:int = true\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_implicit_conversion_int_to_bool) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:bool = 13\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_u64_to_u8_implicit_conversion_warning) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64 = 9999\n"
        "b:u8\n"
        "b = a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit conversion from \"u64\" to \"u8\" during assignment.",
            2, 0, 2, 4)
    };

    ck_assert_typecheck_with_messages(true, messages);
} END_TEST

START_TEST (test_typecheck_u32_to_u8_implicit_conversion_warning) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u32 = 9999\n"
        "b:u8\n"
        "b = a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit conversion from \"u32\" to \"u8\" during assignment.",
            2, 0, 2, 4)
    };

    ck_assert_typecheck_with_messages(true, messages);
} END_TEST

START_TEST (test_typecheck_u16_to_u8_implicit_conversion_warning) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u16 = 9999\n"
        "b:u8\n"
        "b = a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit conversion from \"u16\" to \"u8\" during assignment.",
            2, 0, 2, 4)
    };

    ck_assert_typecheck_with_messages(true, messages);
} END_TEST

START_TEST (test_typecheck_signed_to_unsigned_implicit_conversion_warning) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i16 = 9999\n"
        "b:u16\n"
        "b = a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit signed to unsigned conversion from \"i16\" to \"u16\" during assignment.",
            2, 0, 2, 4)
    };

    ck_assert_typecheck_with_messages(true, messages);
} END_TEST

START_TEST (test_typecheck_inv_implicit_conversion_warning) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64 = 9009999\n"
        "b:u16\n"
        "b = a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit signed to unsigned conversion from \"i64\" to \"u16\" during assignment.",
            2, 0, 2, 4),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_WARNING,
            "Implicit conversion from \"i64\" to \"u16\" during assignment.",
            2, 0, 2, 4)
    };

    ck_assert_typecheck_with_messages(true, messages);
} END_TEST

START_TEST (test_typecheck_inv_implicit_conversion_u64_const_to_u8_error) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "b:u8 = 999999999\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign \"u32\" to \"u8\". "
            "Unable to convert from \"u32\" to \"u8\". "
            "Attempting to assign larger constant to smaller variable.",
            1, 0, 1, 15),
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64 = 9009999\n"
        "b:u8\n"
        "b = u8(a)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64 = 9009999\n"
        "b:u8\n"
        "b = u8(a)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:f64 = 4.234\n"
        "b:u8\n"
        "b = u8(a)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion_int_literal_to_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:string\n"
        "a = string(32)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion_float_literal_to_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:string\n"
        "a = string(0.2342)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_explicit_conversion_bool_to_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:string\n"
        "a = string(true)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_invalid_explicit_conversion_empty) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u64\n"
        "a = u64()\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Invalid arguments for explicit conversion to \"u64\".",
            2, 4, 2, 8)
    };
    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST (test_typecheck_invalid_explicit_conversion_int_to_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:string\n"
        "b:u64 = 23412\n"
        "a = string(b)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Invalid explicit conversion. Unable to convert from \"u64\" to \"string\".",
            3, 4, 3, 12)
    };
    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

Suite *analyzer_typecheck_conversion_suite_create(void)
{
    Suite *s = suite_create("analyzer_typecheck_conversions");

    TCase *t_implicit_val = tcase_create("analyzer_typecheck_implicit_conversions");
    tcase_add_checked_fixture(t_implicit_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_implicit_val, test_typecheck_implicit_conversion_bool_to_int);
    tcase_add_test(t_implicit_val, test_typecheck_implicit_conversion_int_to_bool);

    TCase *t_implicit_inv = tcase_create("analyzer_typecheck_invalid_implicit_conversions");
    tcase_add_checked_fixture(t_implicit_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_implicit_inv, test_typecheck_u64_to_u8_implicit_conversion_warning);
    tcase_add_test(t_implicit_inv, test_typecheck_u32_to_u8_implicit_conversion_warning);
    tcase_add_test(t_implicit_inv, test_typecheck_u16_to_u8_implicit_conversion_warning);
    tcase_add_test(t_implicit_inv, test_typecheck_signed_to_unsigned_implicit_conversion_warning);
    tcase_add_test(t_implicit_inv, test_typecheck_inv_implicit_conversion_warning);
    tcase_add_test(t_implicit_inv, test_typecheck_inv_implicit_conversion_u64_const_to_u8_error);


    TCase *t_explicit_conversion_val = tcase_create("analyzer_typecheck_valid_explicit_conversion");
    tcase_add_checked_fixture(t_explicit_conversion_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion1);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion2);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion3);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion_int_literal_to_string);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion_float_literal_to_string);
    tcase_add_test(t_explicit_conversion_val, test_typecheck_valid_explicit_conversion_bool_to_string);

    TCase *t_explicit_conversion_inv = tcase_create("analyzer_typecheck_invalid_explicit_conversion");
    tcase_add_checked_fixture(t_explicit_conversion_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_explicit_conversion_inv, test_typecheck_invalid_explicit_conversion_empty);
    tcase_add_test(t_explicit_conversion_inv, test_typecheck_invalid_explicit_conversion_int_to_string);


    suite_add_tcase(s, t_implicit_val);
    suite_add_tcase(s, t_implicit_inv);
    suite_add_tcase(s, t_explicit_conversion_val);
    suite_add_tcase(s, t_explicit_conversion_inv);
    return s;
}
