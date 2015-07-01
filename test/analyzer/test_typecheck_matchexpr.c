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

#include <types/type.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_typecheck_matchexpr_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32 | s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    i32 => \"number\"\n"
        "    _   => \"other\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_matchexpr_simple_product_of_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    i32, bool => \"number\"\n"
        "    _   => \"other\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_matchexpr_simple_2_wildcards) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    _, _ => \"number or bool\"\n"
        "    _    => \"string\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_matchexpr_simple_3_wildcards) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool, c:string |\n"
        "          d:u64, e:f64            |\n"
        "          f:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    _       => \"case 1\"\n"
        "    _, _    => \"case 2\"\n"
        "    _, _, _ => \"case 3\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_matchexpr_assign_to_check_type_single) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:string = match a {\n"
        "    _       => \"case 1\"\n"
        "    _, _    => \"case 2\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_matchexpr_assign_to_check_type_sum_of_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:(string | i32) = match a {\n"
        "    _       => \"string\"\n"
        "    _, _    => 56\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_access_field) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32 | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:i32 = match a {\n"
        "    z:i32       => z\n"
        "    _    => 0\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();    
} END_TEST

START_TEST (test_typecheck_access_field_same_name_as_parent_block) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32 | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:i32 = match a {\n"
        "    a:i32       => a\n"
        "    _    => 0\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();    
} END_TEST

START_TEST (test_typecheck_access_fieldname_in_typeop) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:f32 | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:f32 = match a {\n"
        "    a:i32, b:f32       => b\n"
        "    string    => 3.14   \n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();    
} END_TEST

START_TEST(test_typecheck_matchexpr_inv_nonexisting_single_case) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    i32 => \"number\"\n"
        "    _   => \"other\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"i32\" can not be matched to the type of \"a\" which is "
            "of type \"foo {string}\".",
            4, 4, 4, 18)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typecheck_matchexpr_inv_nonexisting_case_product_of_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    _, i32 => \"number\"\n"
        "    _, _   => \"other\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"_,i32\" can not be matched to the type of \"a\" which "
            "is of type \"foo {i32,string}\".",
            4, 4, 4, 21)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typecheck_matchexpr_inv_too_many_wildcards) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    _       => \"case 1\"\n"
        "    _, _    => \"case 2\"\n"
        "    _, _, _ => \"case 3\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"_,_,_\" can not be matched to the type of \"a\" which "
            "is of type \"foo {i32,bool|string}\".",
            6, 4, 6, 22)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typecheck_matchexpr_inv_not_all_cases_covered) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32        |\n"
        "          b:u32, c:f32 |\n"
        "          d:i8, e:f32, f:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    i32    => \"number\"\n"
        "    _, _   => \"product of 2\"\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Match expression does not match all cases for \"a\". Sum type "
            "operand of \"i8,f32,string\" is not covered.",
            5, 4, 8, 4)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typecheck_matchexpr_inv_catchall_before_other_cases) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32 | b:string }\n"
        "{\n"
        "    t2:foo = foo(\"hello\")\n"
        "    s:string = match t2 {\n"
        "        _ => \"\"\n"
        "        b:string => b\n"
        "    }\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"string\" is useless since all parts of \"t2\" have "
            "already been matched.",
            5, 8, 5, 20)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

Suite *analyzer_typecheck_matchexpr_suite_create(void)
{
    Suite *s = suite_create("typecheck_match_expressions");

    TCase *t_simple = tcase_create("simple_match_expressions");
    tcase_add_checked_fixture(t_simple,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_simple, test_typecheck_matchexpr_simple);
    tcase_add_test(t_simple, test_typecheck_matchexpr_simple_product_of_2);
    tcase_add_test(t_simple, test_typecheck_matchexpr_simple_2_wildcards);
    tcase_add_test(t_simple, test_typecheck_matchexpr_simple_3_wildcards);
    tcase_add_test(t_simple, test_typecheck_matchexpr_assign_to_check_type_single);
    tcase_add_test(t_simple, test_typecheck_matchexpr_assign_to_check_type_sum_of_2);

    TCase *t_advanced = tcase_create("advanced_match_expressions");
    tcase_add_checked_fixture(t_advanced,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_advanced, test_typecheck_access_field);
    tcase_add_test(t_advanced, test_typecheck_access_field_same_name_as_parent_block);
    tcase_add_test(t_advanced, test_typecheck_access_fieldname_in_typeop);

    TCase *t_inv = tcase_create("invalid_match_expressions");
    tcase_add_checked_fixture(t_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_nonexisting_single_case);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_nonexisting_case_product_of_2);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_too_many_wildcards);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_not_all_cases_covered);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_catchall_before_other_cases);

    suite_add_tcase(s, t_simple);
    suite_add_tcase(s, t_advanced);
    suite_add_tcase(s, t_inv);
    return s;
}


