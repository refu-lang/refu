/**
 * Tests for parsing of ast node not having their own test file
 */

#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/block.h"
#include <ast/function.h>
#include <ast/type.h>
#include <ast/typeclass.h>
#include <ast/generics.h>
#include <ast/block.h>
#include <ast/operators.h>
#include <ast/constants.h>
#include <ast/import.h>
#include <ast/string_literal.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST (test_acc_string_literals) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a = \"a_string_literal\"\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 1, 0, 1, 0);
    testsupport_parser_string_literal_create(lit_1, file, 1, 4, 1, 21);
    testsupport_parser_node_create(bop1, binaryop, file, 1, 0, 1, 21,
                                   BINARYOP_ASSIGN, id_a, lit_1);


    testsupport_parser_block_create(bnode, file, 0, 0, 2, 0);
    ast_node_add_child(bnode, bop1);

    ck_test_parse_as(n, block, d, "block with literals", bnode, true);

    // check that the string literal gets parsed without the \"\"
    struct ast_node *c;
    // get first child TODO: (kinda stupid way maybe give accessors?)
    rf_ilist_for_each(&n->children, c, lh) {
        break;
    }
    ck_assert_rf_str_eq_cstr(ast_string_literal_get_str(ast_binaryop_right(c)),
                             "a_string_literal");

    ast_node_destroy(n);
    ast_node_destroy(bnode);
} END_TEST

START_TEST (test_acc_boolean_constants) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a = true\n"
        "b = false\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 1, 0, 1, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(file,
                                                                 2, 0, 2, 0);
    testsupport_parser_constant_create(t_node, file, 1, 4, 1, 7, boolean, true);
    testsupport_parser_constant_create(f_node, file, 2, 4, 2, 8, boolean, false);
    testsupport_parser_node_create(bop1, binaryop, file, 1, 0, 1, 7,
                                   BINARYOP_ASSIGN, id_a, t_node);
    testsupport_parser_node_create(bop2, binaryop, file, 2, 0, 2, 8,
                                   BINARYOP_ASSIGN, id_b, f_node);

    testsupport_parser_block_create(bnode, file, 0, 0, 3, 0);
    ast_node_add_child(bnode, bop1);
    ast_node_add_child(bnode, bop2);

    ck_test_parse_as(n, block, d, "block with boolean constants", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
} END_TEST

START_TEST (test_acc_import_statements) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "import mod1, mod2\n"
        "import mod3\n"
        "foreign_import a_function\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_mod1 = testsupport_parser_identifier_create(file,
                                                                    0, 7, 0, 10);
    struct ast_node *id_mod2 = testsupport_parser_identifier_create(file,
                                                                    0, 13, 0, 16);
    struct ast_node *id_mod3 = testsupport_parser_identifier_create(file,
                                                                    1, 7, 1, 10);
    struct ast_node *id_fn = testsupport_parser_identifier_create(file,
                                                                  2, 15, 2, 24);
    testsupport_parser_node_create(imp1, import, file, 0, 0, 0, 16, false);
    ast_node_add_child(imp1, id_mod1);
    ast_node_add_child(imp1, id_mod2);
    testsupport_parser_node_create(imp2, import, file, 1, 0, 1, 10, false);
    ast_node_add_child(imp2, id_mod3);
    testsupport_parser_node_create(imp3, import, file, 2, 0, 2, 24, true);
    ast_node_add_child(imp3, id_fn);

    struct ast_node *expected_root = ast_root_create(file);
    ast_node_add_child(expected_root, imp1);
    ast_node_add_child(expected_root, imp2);
    ast_node_add_child(expected_root, imp3);

    ck_test_parse_root(n, d, expected_root);

    ast_node_destroy(n);
    ast_node_destroy(expected_root);
} END_TEST

START_TEST (test_acc_import_statements_fail1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("import ");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    ck_assert_msg(!parser_process_file(d->front.parser), "parsing should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            d->front.file,
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier at import statement",
            0, 5),
        TESTSUPPORT_INFOMSG_INIT_START(
            d->front.file,
            MESSAGE_SYNTAX_ERROR,
            "Expected an outermost statement",
            0, 0),
    };
    ck_assert_parser_errors(d->front.info, errors);

} END_TEST

START_TEST (test_acc_import_statements_fail2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("foreign_import func1,");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    ck_assert_msg(!parser_process_file(d->front.parser), "parsing should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            d->front.file,
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier at foreign_import statement",
            0, 20),
        TESTSUPPORT_INFOMSG_INIT_START(
            d->front.file,
            MESSAGE_SYNTAX_ERROR,
            "Expected an outermost statement",
            0, 15),
    };
    ck_assert_parser_errors(d->front.info, errors);

} END_TEST

Suite *parser_misc_suite_create(void)
{
    Suite *s = suite_create("parser_misc");

    TCase *tc1 = tcase_create("parser_misc_nodes");
    tcase_add_checked_fixture(tc1, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc1, test_acc_string_literals);
    tcase_add_test(tc1, test_acc_boolean_constants);
    tcase_add_test(tc1, test_acc_import_statements);

    tcase_add_test(tc1, test_acc_import_statements_fail1);
    tcase_add_test(tc1, test_acc_import_statements_fail2);

    suite_add_tcase(s, tc1);
    return s;
}
