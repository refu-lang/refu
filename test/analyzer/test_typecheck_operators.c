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
        "c:bool\n"
        "a = 456\n"
        "name = \"Lefteris\"\n"
        "b = 0.231\n"
        "c = false\n"
        "c = true\n"
        "c = a > 100\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
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
            "\"string\" to \"u64\". Unable to convert from \"string\" to \"u64\".",
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

START_TEST (test_typecheck_valid_uop_minus) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = -a\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_uop_plus) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = +a\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_uop_inc_pre) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = ++a\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_uop_inc_post) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = a++\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_uop_dec_pre) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = --a\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_valid_uop_dec_post) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:i64\n"
        "b:i64 = a--\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);
} END_TEST

START_TEST (test_typecheck_invalid_uop_minus) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "b:string = -\"foo\"\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Can't apply \"-\" to \"string\"",
            1, 11, 1, 16),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            1, 11, 1, 16),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_invalid_uop_plus) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:bool\n"
        "b:i64 = +a\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Can't apply \"+\" to \"bool\"",
            1, 8, 1, 9),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            1, 8, 1, 9),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_invalid_uop_inc_pre) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "b:i64 = ++\"foo\"\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Can't apply \"++\" to \"string\"",
            1, 8, 1, 14),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            1, 8, 1, 14),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

START_TEST (test_typecheck_invalid_uop_dec_post) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "b:i64 = \"foo\"--\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Can't apply \"--\" to \"string\"",
            1, 8, 1, 14),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            1, 8, 1, 14),
    };
    ck_assert_typecheck_with_messages(d, false, messages, true);
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

START_TEST(test_typecheck_invalid_member_access2) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person {name:string, age:u32}\n"
        "{\n"
        "p:person\n"
        "a:u32 = 42\n"
        "b:u32 = a + s.name\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Undeclared identifier \"s\" as left part of member access operator",
            4, 12, 4, 12),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"+\" can not be determined",
            4, 12, 4, 17),
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            d->front.file,
            MESSAGE_SEMANTIC_ERROR,
            "Type of right side of \"=\" can not be determined",
            4, 8, 4, 17),
    };

    ck_assert_typecheck_with_messages(d, false, messages, true);
} END_TEST

Suite *analyzer_typecheck_operators_suite_create(void)
{
    Suite *s = suite_create("typecheck_operators");

    TCase *t_assign_val = tcase_create("typecheck_assignments");
    tcase_add_checked_fixture(t_assign_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_assign_val, test_typecheck_assignment_simple);

    TCase *t_assign_inv = tcase_create("typecheck_invalid_assignments");
    tcase_add_checked_fixture(t_assign_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_assign_inv, test_typecheck_assignment_invalid_string_to_int);

    TCase *t_bop_val = tcase_create("typecheck_binary_operations");
    tcase_add_checked_fixture(t_bop_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_bop_val, test_typecheck_valid_addition_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_subtraction_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_multiplication_simple);
    tcase_add_test(t_bop_val, test_typecheck_valid_division_simple);

    TCase *t_uop_val = tcase_create("typecheck_valid_unary_operations");
    tcase_add_checked_fixture(t_uop_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_minus);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_plus);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_inc_pre);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_inc_post);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_dec_pre);
    tcase_add_test(t_uop_val, test_typecheck_valid_uop_dec_post);

    TCase *t_uop_inv = tcase_create("typecheck_invalid_unary_operations");
    tcase_add_checked_fixture(t_uop_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_uop_inv, test_typecheck_invalid_uop_minus);
    tcase_add_test(t_uop_inv, test_typecheck_invalid_uop_plus);
    tcase_add_test(t_uop_inv, test_typecheck_invalid_uop_inc_pre);
    tcase_add_test(t_uop_inv, test_typecheck_invalid_uop_dec_post);

    TCase *t_access_val = tcase_create("typecheck_valid_access_operations");
    tcase_add_checked_fixture(t_access_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_access_val, test_typecheck_valid_member_access);

    TCase *t_access_inv = tcase_create("typecheck_invalid_access_operations");
    tcase_add_checked_fixture(t_access_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_access_inv, test_typecheck_invalid_member_access);
    tcase_add_test(t_access_inv, test_typecheck_invalid_member_access2);


    suite_add_tcase(s, t_assign_val);
    suite_add_tcase(s, t_assign_inv);
    suite_add_tcase(s, t_bop_val);
    suite_add_tcase(s, t_uop_val);
    suite_add_tcase(s, t_uop_inv);
    suite_add_tcase(s, t_access_val);
    suite_add_tcase(s, t_access_inv);

    return s;
}


