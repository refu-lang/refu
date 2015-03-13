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

Suite *parser_expressions_suite_create(void)
{
    Suite *s = suite_create("parser_expressions");

    TCase *tc1 = tcase_create("parser_general_expressions");
    tcase_add_checked_fixture(tc1, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc1, test_acc_string_literals);
    tcase_add_test(tc1, test_acc_boolean_constants);

    suite_add_tcase(s, tc1);
    return s;
}
