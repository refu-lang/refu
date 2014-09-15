#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/generics.h"
#include <ast/ast.h>
#include <lexer/lexer.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_genrdecl_simple1) {
    struct ast_node *n;
    struct front_ctx *front;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type a>");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    file = &front->file;
    ck_assert_msg(front, "Failed to assign string to file ");

    testsupport_parser_identifier_create(id1, file, 0, 1, 0, 4);
    testsupport_parser_identifier_create(id2, file, 0, 6, 0, 6);
    struct ast_node *gtype1 = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, file, 0, 0, 0, 7);
    ast_node_add_child(genr, gtype1);

    ck_test_parse_as(n, genrdecl, d, "generic type declaration", genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST


START_TEST(test_acc_genrdecl_simple2) {
    struct ast_node *n;
    struct front_ctx *front;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  <  Type a >  ");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    file = &front->file;
    ck_assert_msg(front, "Failed to assign string to file ");

    testsupport_parser_identifier_create(id1, file, 0, 5, 0, 8);
    testsupport_parser_identifier_create(id2, file, 0, 10, 0, 10);
    struct ast_node *gtype1 = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, file, 0, 2, 0, 12);
    ast_node_add_child(genr, gtype1);

    ck_test_parse_as(n, genrdecl, d, "generic type declaration", genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST


START_TEST(test_acc_genrdecl_simple3) {
    struct ast_node *n;
    struct front_ctx *front;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type a, Type b>");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    file = &front->file;
    ck_assert_msg(front, "Failed to assign string to file ");


    testsupport_parser_identifier_create(id1, file, 0, 1, 0, 4);
    testsupport_parser_identifier_create(id2, file, 0, 6, 0, 6);
    struct ast_node *gtype1 = ast_genrtype_create(id1, id2);

    testsupport_parser_identifier_create(id3, file, 0, 9, 0, 12);
    testsupport_parser_identifier_create(id4, file, 0, 14, 0, 14);
    struct ast_node *gtype2 = ast_genrtype_create(id3, id4);
    testsupport_parser_node_create(genr, genrdecl, file, 0, 0, 0, 15);
    ast_node_add_child(genr, gtype1);
    ast_node_add_child(genr, gtype2);

    ck_test_parse_as(n, genrdecl, d, "generic type declaration", genr);

    ast_node_destroy(n);
    ast_node_destroy(genr);
} END_TEST


START_TEST(test_acc_genrdecl_fail1) {
    struct ast_node *n;
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type ");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");

    testsupport_parser_prepare(d);
    n = parser_acc_genrdecl(front->parser);
    ck_assert_msg(n == NULL, "parsing generic declaration should fail");
    ck_assert_parser_error(
        d,
        "test_file:0:6 error: Expected an identifier for the generic type name\n"
        "<type \n"
        "      ^\n"
    );
} END_TEST

START_TEST(test_acc_genrdecl_fail2) {
    struct ast_node *n;
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type a bbb");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");

    testsupport_parser_prepare(d);
    n = parser_acc_genrdecl(front->parser);
    ck_assert_msg(n == NULL, "parsing generic declaration should fail");
    ck_assert_parser_error(
        d,
        "test_file:0:8 error: "
        "Expected either a ',' or a '>' at generic declaration\n"
        "<type a bbb\n"
        "        ^\n"
    );

} END_TEST


Suite *parser_generics_suite_create(void)
{
    Suite *s = suite_create("parser_generics");

    TCase *genrdecl = tcase_create("parser_generics_genrdecl");
    tcase_add_checked_fixture(genrdecl,
                              setup_front_tests,
                              teardown_front_tests);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple1);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple2);
    tcase_add_test(genrdecl, test_acc_genrdecl_simple3);

    tcase_add_test(genrdecl, test_acc_genrdecl_fail1);
    tcase_add_test(genrdecl, test_acc_genrdecl_fail2);

    suite_add_tcase(s, genrdecl);

    return s;
}


