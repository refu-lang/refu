#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/expression.h"
#include <ast/string_literal.h>
#include <ast/constant_num.h>
#include <ast/operators.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_addition) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a + b");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 0);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 4, 0, 4);
    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 4,
                                   BINARYOP_ADD,
                                   id1, id2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_multi) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "214 * foo");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    testsupport_parser_constant_create(cnum, file,
                                       0, 0, 0, 2, integer, 214);
    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 6, 0, 8);
    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 8,
                                   BINARYOP_MUL,
                                   cnum, id1);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_sub) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "214 - 5651");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    testsupport_parser_constant_create(cnum1, file,
                                       0, 0, 0, 2, integer, 214);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 6, 0, 9, integer, 5651);
    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 9,
                                   BINARYOP_SUB,
                                   cnum1, cnum2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_div) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "3.142/2.1245");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    testsupport_parser_constant_create(cnum1, file,
                                       0, 0, 0, 4, float, 3.142);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 6, 0, 11, float, 2.1245);
    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 11,
                                   BINARYOP_DIV,
                                   cnum1, cnum2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

Suite *parser_operators_suite_create(void)
{
    Suite *s = suite_create("parser_operators");

    TCase *bop = tcase_create("parser_operators_binary");
    tcase_add_checked_fixture(bop, setup_front_tests, teardown_front_tests);
    tcase_add_test(bop, test_acc_addition);
    tcase_add_test(bop, test_acc_multi);
    tcase_add_test(bop, test_acc_sub);
    tcase_add_test(bop, test_acc_div);

    suite_add_tcase(s, bop);
    return s;
}
