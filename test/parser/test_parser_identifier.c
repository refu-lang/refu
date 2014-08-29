#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_identifier_spaced) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring id_string = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring s = RF_STRING_STATIC_INIT("  foo ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n, "Could not parse identifier");
    ck_assert_ast_node_loc(n, 0, 2, 0, 5);
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "foo");
    ast_node_destroy(n);
}END_TEST

START_TEST(test_acc_identifier_narrow) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring id_string = RF_STRING_STATIC_INIT("narrow");
    static const struct RFstring s = RF_STRING_STATIC_INIT("narrow");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n, "Could not parse identifier");
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "narrow");
    ck_assert_ast_node_loc(n, 0, 0, 0, 5);
    ast_node_destroy(n);
}END_TEST

START_TEST(test_acc_identifier_fail1) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("-x1-x");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n == NULL, "Accepting identifier should have failed");
}END_TEST

START_TEST(test_acc_identifier_fail2) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  \n \n \r   ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(!n, "Accepting identifier should have failed");
}END_TEST

START_TEST(test_acc_identifier_fail3) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(!n, "Accepting identifier should have failed");
}END_TEST


Suite *parser_identifier_suite_create(void)
{
    Suite *s = suite_create("Parser - Identifier");

    TCase *id1 = tcase_create("identifiers");
    tcase_add_checked_fixture(id1, setup_parser_tests, teardown_parser_tests);
    tcase_add_test(id1, test_acc_identifier_spaced);
    tcase_add_test(id1, test_acc_identifier_narrow);

    tcase_add_test(id1, test_acc_identifier_fail1);
    tcase_add_test(id1, test_acc_identifier_fail2);
    tcase_add_test(id1, test_acc_identifier_fail3);


    suite_add_tcase(s, id1);
    return s;
}


