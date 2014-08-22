#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

//TODO: fix paths for such includes
#include "../../clib/test/test_helpers.h"

START_TEST(test_acc_identifier) {
    struct ast_node *n;
    static const struct RFstring id_string = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring s = RF_STRING_STATIC_INIT("  foo ");
    struct parser_file *f = parser_file_dummy_get();
    ck_assert_msg(parser_file_dummy_assign(f, &s), "Failed to assign string");

    n = parser_file_acc_identifier(f);
    ck_assert_msg(n, "Could not parse identifier");
    ck_assert_ast_node_loc(n, 0, 2, 0, 5);
    ck_assert_rf_str_eq_cstr(ast_identifier_str(n), "foo");
    ast_node_destroy(n);
}END_TEST


Suite *parser_identifier_suite_create(void)
{
    Suite *s = suite_create("Parser - Identifier");

    TCase *p1 = tcase_create("accepting identifiers");
    tcase_add_checked_fixture(p1, setup_parser_tests, teardown_parser_tests);
    tcase_add_test(p1, test_acc_identifier);


    suite_add_tcase(s, p1);
    return s;
}


