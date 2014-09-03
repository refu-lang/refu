#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include <parser/identifier.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_ws_simple) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  asd    ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    parser_file_acc_ws(f);
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 2, 2, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "asd    ");
} END_TEST

START_TEST(test_acc_ws_simple_nl) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        " \n \r \t asd \n   ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    parser_file_acc_ws(f);
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 7, 7, 1);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "asd \n   ");
} END_TEST

START_TEST(test_acc_ws_full) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(" \n \r \t \n \n  ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    parser_file_acc_ws(f);
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 12, 12, 3);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "");
} END_TEST

START_TEST(test_acc_ws_none) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "identifier_before_space \n \r \t");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    parser_file_acc_ws(f);
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 0, 0, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "identifier_before_space \n \r \t");
} END_TEST

START_TEST(test_acc_ws_none_empty) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT("");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    parser_file_acc_ws(f);
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 0, 0, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "");
} END_TEST


START_TEST(test_acc_string_ascii_1) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring target = RF_STRING_STATIC_INIT("target");
    static const struct RFstring s = RF_STRING_STATIC_INIT("target ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    ck_assert_msg(parser_file_acc_string_ascii(f, &target),
                  "could not accept string");
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 6, 6, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, " ");
} END_TEST

START_TEST(test_acc_string_ascii_2) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring target = RF_STRING_STATIC_INIT(",");
    static const struct RFstring s = RF_STRING_STATIC_INIT(",foo ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    ck_assert_msg(parser_file_acc_string_ascii(f, &target),
                  "could not accept string");
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 1, 1, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "foo ");
} END_TEST

START_TEST(test_acc_string_ascii_3) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring target = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring s = RF_STRING_STATIC_INIT("foo");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    ck_assert_msg(parser_file_acc_string_ascii(f, &target),
                  "could not accept string");
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 3, 3, 0); //move after all 3 bytes to EOF
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "");
} END_TEST

START_TEST(test_acc_string_ascii_fail_1) {
    struct parser_file *f;
    struct parser_offset *off;
    struct RFstringx *str;
    static const struct RFstring target = RF_STRING_STATIC_INIT("target");
    static const struct RFstring s = RF_STRING_STATIC_INIT("  target ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    ck_assert_msg(!parser_file_acc_string_ascii(f, &target),
                  "accepting string should have failed");
    off = parser_file_offset(f);
    ck_assert_parser_offset_eq(off, 0, 0, 0);
    str = parser_file_str(f);
    ck_assert_rf_str_eq_cstr(str, "  target ");
} END_TEST




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
    ck_assert_ast_node_loc(n, 0, 2, 0, 4);
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "foo");
    ck_assert_rf_str_eq_cstr(parser_file_str(f), " ");
    ast_node_destroy(n);
} END_TEST

START_TEST(test_acc_identifier_comma) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring id_string = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring s = RF_STRING_STATIC_INIT("  foo, ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n, "Could not parse identifier");
    ck_assert_ast_node_loc(n, 0, 2, 0, 4);
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "foo");
    ck_assert_rf_str_eq_cstr(parser_file_str(f), ", ");
    ast_node_destroy(n);
} END_TEST

START_TEST(test_acc_identifier_onechar) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring id_string = RF_STRING_STATIC_INIT("a");
    static const struct RFstring s = RF_STRING_STATIC_INIT("   a: ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n, "Could not parse identifier");
    ck_assert_ast_node_loc(n, 0, 3, 0, 3);
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "a");
    ck_assert_rf_str_eq_cstr(parser_file_str(f), ": ");
    ast_node_destroy(n);
} END_TEST

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
    ck_assert_rf_str_eq_cstr(parser_file_str(f), "");
    ast_node_destroy(n);
} END_TEST

START_TEST(test_acc_identifier_fail1) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("-x1-x");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n == NULL, "Accepting identifier should have failed");
    ck_assert_driver_offset_eq(d, 0, 0, 0);
} END_TEST

START_TEST(test_acc_identifier_fail2) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  \n \n \r   ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(!n, "Accepting identifier should have failed");
    ck_assert_driver_offset_eq(d, 0, 0, 0);
} END_TEST

START_TEST(test_acc_identifier_fail3) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(!n, "Accepting identifier should have failed");
    ck_assert_driver_offset_eq(d, 0, 0, 0);
} END_TEST


Suite *parser_base_suite_create(void)
{
    Suite *s = suite_create("parser_base");

    TCase *whitespace = tcase_create("parser_base_whitespace");
    tcase_add_checked_fixture(whitespace,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(whitespace, test_acc_ws_simple);
    tcase_add_test(whitespace, test_acc_ws_simple_nl);
    tcase_add_test(whitespace, test_acc_ws_full);
    tcase_add_test(whitespace, test_acc_ws_none);
    tcase_add_test(whitespace, test_acc_ws_none_empty);

    TCase *accept_string = tcase_create("parser_base_accept_string");
    tcase_add_checked_fixture(accept_string,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(accept_string, test_acc_string_ascii_1);
    tcase_add_test(accept_string, test_acc_string_ascii_2);
    tcase_add_test(accept_string, test_acc_string_ascii_3);
    tcase_add_test(accept_string, test_acc_string_ascii_fail_1);

    TCase *identifiers = tcase_create("parser_base_identifiers");
    tcase_add_checked_fixture(identifiers,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(identifiers, test_acc_identifier_spaced);
    tcase_add_test(identifiers, test_acc_identifier_comma);
    tcase_add_test(identifiers, test_acc_identifier_narrow);
    tcase_add_test(identifiers, test_acc_identifier_onechar);

    tcase_add_test(identifiers, test_acc_identifier_fail1);
    tcase_add_test(identifiers, test_acc_identifier_fail2);
    tcase_add_test(identifiers, test_acc_identifier_fail3);


    suite_add_tcase(s, whitespace);
    suite_add_tcase(s, accept_string);
    suite_add_tcase(s, identifiers);
    return s;
}


