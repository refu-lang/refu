#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include <parser/identifier.h>
#include <parser/generics.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_genrdecl_simple1) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<type a>");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_p(f);

    struct ast_node *id1 = ast_identifier_create(f, sp + 1, sp + 4);
    struct ast_node *id2 = ast_identifier_create(f, sp + 6, sp + 6);
    struct ast_node *genr = ast_genrdecl_create(f, sp, sp + 7);
    ast_node_add_child(genr, ast_genrtype_create(f, sp + 1, sp + 6, id1, id2));

    n = parser_file_acc_genrdecl(f);
    ck_assert_parsed_node(n, d, "Could not parse generic type declaration");
    check_ast_match(n, genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST

START_TEST(test_acc_genrdecl_simple2) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  <  type a >  ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_p(f);

    struct ast_node *id1 = ast_identifier_create(f, sp + 5, sp + 8);
    struct ast_node *id2 = ast_identifier_create(f, sp + 10, sp + 10);
    struct ast_node *genr = ast_genrdecl_create(f, sp + 2, sp + 12);
    ast_node_add_child(genr, ast_genrtype_create(f, sp + 5 , sp + 10, id1, id2));

    n = parser_file_acc_genrdecl(f);
    ck_assert_parsed_node(n, d, "Could not parse generic type declaration");
    check_ast_match(n, genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST

START_TEST(test_acc_genrdecl_simple3) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<type a, type b>");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_p(f);

    struct ast_node *id1 = ast_identifier_create(f, sp + 1, sp + 4);
    struct ast_node *id2 = ast_identifier_create(f, sp + 6, sp + 6);
    struct ast_node *genrtype1 = ast_genrtype_create(f, sp + 1,
                                                     sp + 6, id1, id2);
    struct ast_node *id3 = ast_identifier_create(f, sp + 9, sp + 12);
    struct ast_node *id4 = ast_identifier_create(f, sp + 14, sp + 14);
    struct ast_node *genrtype2 = ast_genrtype_create(f, sp + 9,
                                                     sp + 14, id3, id4);
    struct ast_node *genr = ast_genrdecl_create(f, sp, sp + 15);
    ast_node_add_child(genr, genrtype1);
    ast_node_add_child(genr, genrtype2);

    n = parser_file_acc_genrdecl(f);
    ck_assert_parsed_node(n, d, "Could not parse generic type declaration");
    check_ast_match(n, genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST



START_TEST(test_acc_genrdecl_fail1) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<type ");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_genrdecl(f);
    ck_assert_msg(n == NULL, "parsing generic declaration should fail");
    ck_assert_parser_error(
        d,
        "test_file:0:6 error: Expected an identifier for the generic type name\n"
        "<type \n"
        "      ^\n"
    );

    ck_assert_driver_offset_eq(d, 0, 0, 0);
    ck_assert_rf_str_eq_cstr(parser_file_str(f), "<type ");
} END_TEST

START_TEST(test_acc_genrdecl_fail2) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<type a bbb");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_genrdecl(f);
    ck_assert_msg(n == NULL, "parsing generic declaration should fail");
    ck_assert_parser_error(
        d,
        "test_file:0:8 error: "
        "Expected either a ',' or a '>' at generic declaration\n"
        "<type a bbb\n"
        "        ^\n"
    );

    ck_assert_driver_offset_eq(d, 0, 0, 0);
    ck_assert_rf_str_eq_cstr(parser_file_str(f), "<type a bbb");
} END_TEST



Suite *parser_generics_suite_create(void)
{
    Suite *s = suite_create("parser_generics");

    TCase *genrdecl = tcase_create("parser_generics_genrdecl");
    tcase_add_checked_fixture(genrdecl,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple1);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple2);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple3);

    tcase_add_test(genrdecl, test_acc_genrdecl_fail1);
    tcase_add_test(genrdecl, test_acc_genrdecl_fail2);

    suite_add_tcase(s, genrdecl);

    return s;
}


