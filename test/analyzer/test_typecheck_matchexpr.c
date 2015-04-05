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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST(test_typecheck_matchexpr_assign_to_check_type_sum_of_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, b:bool | c:string }\n"
        "{\n"
        "    a:foo\n"
        "    s:(i32 | string) = match a {\n"
        "    _       => \"string\"\n"
        "    _, _    => 56\n"
        "    }\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"i32\" can not be matched to the type of \"a\" which is "
            "of type \"foo { string }\".",
            4, 4, 4, 18)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"_,i32\" can not be matched to the type of \"a\" which "
            "is of type \"foo { i32,string }\".",
            4, 4, 4, 21)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Match case \"_,_,_\" can not be matched to the type of \"a\" which "
            "is of type \"foo { i32,bool|string }\".",
            6, 4, 6, 22)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Match expression does not match all cases for \"a\". Sum type "
            "operand of \"i8,f32,string\" is not covered.",
            5, 4, 8, 4)
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
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

    TCase *t_inv = tcase_create("invalid_match_expressions");
    tcase_add_checked_fixture(t_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_nonexisting_single_case);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_nonexisting_case_product_of_2);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_too_many_wildcards);
    tcase_add_test(t_inv, test_typecheck_matchexpr_inv_not_all_cases_covered);

    suite_add_tcase(s, t_simple);
    suite_add_tcase(s, t_inv);
    return s;
}


