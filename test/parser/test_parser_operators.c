#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/core.h>

#include <parser/parser.h>
#include "../../src/parser/recursive_descent/expression.h"
#include <ast/string_literal.h>
#include <ast/constants.h>
#include <ast/operators.h>
#include <ast/generics.h>
#include <ast/function.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_addition) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a + b");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 4, 0, 4);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 4,
                                   BINARYOP_ADD,
                                   id1, id2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_multi) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "214 * foo");
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_constant_create(cnum, 0, 0, 0, 2, integer, 214);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 6, 0, 8);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 8,
                                   BINARYOP_MUL,
                                   cnum, id1);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_sub) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "214 - 5651");
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_constant_create(cnum1, 0, 0, 0, 2, integer, 214);
    testsupport_parser_constant_create(cnum2, 0, 6, 0, 9, integer, 5651);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 9,
                                   BINARYOP_SUB,
                                   cnum1, cnum2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_div) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "3.142/2.1245");
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_constant_create(cnum1, 0, 0, 0, 4, float, 3.142);
    testsupport_parser_constant_create(cnum2, 0, 6, 0, 11, float, 2.1245);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 11,
                                   BINARYOP_DIV,
                                   cnum1, cnum2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_assignment) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a = 5 + 2.149");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_constant_create(cnum1, 0, 4, 0, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, 0, 8, 0, 12, float, 2.149);
    testsupport_parser_node_create(bop1, binaryop, 0, 4, 0, 12,
                                   BINARYOP_ADD,
                                   cnum1, cnum2);

    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 12,
                                   BINARYOP_ASSIGN,
                                   id1, bop1);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_minus) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a = -b");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id_a = testsupport_parser_identifier_create(0, 0, 0, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(0, 5, 0, 5);
    testsupport_parser_node_create(uop1, unaryop, 0, 4, 0, 5,
                                   UNARYOP_MINUS,
                                   id_b);

    testsupport_parser_node_create(bop1, binaryop, 0, 0, 0, 5,
                                   BINARYOP_ASSIGN,
                                   id_a, uop1);

    ck_test_parse_as(n, expression, "binary operator", bop1);

    ast_node_destroy(n);
    ast_node_destroy(bop1);
}END_TEST

START_TEST(test_acc_plus) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a = +b");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id_a = testsupport_parser_identifier_create(0, 0, 0, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(0, 5, 0, 5);
    testsupport_parser_node_create(uop1, unaryop, 0, 4, 0, 5,
                                   UNARYOP_PLUS,
                                   id_b);

    testsupport_parser_node_create(bop1, binaryop, 0, 0, 0, 5,
                                   BINARYOP_ASSIGN,
                                   id_a, uop1);

    ck_test_parse_as(n, expression, "binary operator", bop1);

    ast_node_destroy(n);
    ast_node_destroy(bop1);
}END_TEST

START_TEST(test_acc_complex_binary_op_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a + 12.232 * 5 - foo");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_constant_create(cnum1, 0, 4, 0, 9, float, 12.232);
    testsupport_parser_constant_create(cnum2, 0, 13, 0, 13, integer, 5);

    testsupport_parser_node_create(bop1, binaryop, 0, 4, 0, 13,
                                   BINARYOP_MUL,
                                   cnum1, cnum2);
    testsupport_parser_node_create(bop2, binaryop, 0, 0, 0, 13,
                                   BINARYOP_ADD,
                                   id1, bop1);

    struct ast_node *id2 = testsupport_parser_identifier_create(0, 17, 0, 19);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 19,
                                   BINARYOP_SUB,
                                   bop2, id2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "\"start\" + 1.0e-10 - (a + dosth(42, foo))");
    front_testdriver_new_ast_main_source(&s);


    testsupport_parser_string_literal_create(literal, 0, 0, 0, 6);
    testsupport_parser_constant_create(cnum1, 0, 10, 0, 16, float, 1.0e-10);
    testsupport_parser_node_create(bop1, binaryop, 0, 0, 0, 16,
                                   BINARYOP_ADD,
                                   literal, cnum1);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 21, 0, 21);


    struct ast_node *fn_name = testsupport_parser_identifier_create(0, 25, 0, 29);
    testsupport_parser_constant_create(cnum2, 0, 31, 0, 32, integer, 42);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 35, 0, 37);

    testsupport_parser_node_create(arg_bop, binaryop, 0, 31, 0, 37,
                                   BINARYOP_COMMA,
                                   cnum2, id2);
    testsupport_parser_node_create(fn, fncall, 0, 25, 0, 38,
                                   fn_name, arg_bop, NULL);

    testsupport_parser_node_create(bop2, binaryop, 0, 21, 0, 38,
                                   BINARYOP_ADD,
                                   id1, fn);

    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 38,
                                   BINARYOP_SUB,
                                   bop1, bop2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(foo<a, b>((25/2) * 323 + 2) + 325) * 3.14");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *fn_name = testsupport_parser_identifier_create(0, 1, 0, 3);

    testsupport_parser_xidentifier_create_simple(id1, 0, 5, 0, 5);
    testsupport_parser_xidentifier_create_simple(id2, 0, 8, 0, 8);
    testsupport_parser_node_create(gattr, genrattr, 0, 4, 0, 9);
    ast_node_add_child(gattr, id1);
    ast_node_add_child(gattr, id2);

    testsupport_parser_constant_create(cnum1, 0, 12, 0, 13, integer, 25);
    testsupport_parser_constant_create(cnum2, 0, 15, 0, 15, integer, 2);
    testsupport_parser_node_create(bop1, binaryop, 0, 12, 0, 15,
                                   BINARYOP_DIV,
                                   cnum1, cnum2);

    testsupport_parser_constant_create(cnum3, 0, 20, 0, 22, integer, 323);
    testsupport_parser_node_create(bop2, binaryop, 0, 12, 0, 22,
                                   BINARYOP_MUL,
                                   bop1, cnum3);

    testsupport_parser_constant_create(cnum4, 0, 26, 0, 26, integer, 2);
    testsupport_parser_node_create(bop3, binaryop, 0, 12, 0, 26,
                                   BINARYOP_ADD,
                                   bop2, cnum4);

    testsupport_parser_node_create(fn, fncall, 0, 1, 0, 27,
                                   fn_name, bop3, gattr);

    testsupport_parser_constant_create(cnum5, 0, 31, 0, 33, integer, 325);
    testsupport_parser_node_create(bop4, binaryop, 0, 1, 0, 33,
                                   BINARYOP_ADD,
                                   fn, cnum5);

    testsupport_parser_constant_create(cnum6, 0, 38, 0, 41, float, 3.14);
    testsupport_parser_node_create(bop, binaryop, 0, 1, 0, 41,
                                   BINARYOP_MUL,
                                   bop4, cnum6);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_4) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(table[56] + foo(3, b)) + 4 * 321");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 1, 0, 5);
    testsupport_parser_constant_create(cnum1, 0, 7, 0, 8, integer, 56);
    testsupport_parser_node_create(arr, binaryop, 0, 1, 0, 9,
                                   BINARYOP_INDEX_ACCESS, id1, cnum1);

    struct ast_node *fn_name = testsupport_parser_identifier_create(0, 13, 0, 15);
    testsupport_parser_constant_create(cnum2, 0, 17, 0, 17, integer, 3);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 20, 0, 20);
    testsupport_parser_node_create(arg_bop, binaryop, 0, 17, 0, 20,
                                   BINARYOP_COMMA,
                                   cnum2, id2);
    testsupport_parser_node_create(fn, fncall, 0, 13, 0, 21,
                                   fn_name, arg_bop, NULL);

    testsupport_parser_node_create(bop1, binaryop, 0, 1, 0, 21,
                                   BINARYOP_ADD, arr, fn);

    testsupport_parser_constant_create(cnum3, 0, 26, 0, 26, integer, 4);
    testsupport_parser_constant_create(cnum4, 0, 30, 0, 32, integer, 321);
    testsupport_parser_node_create(bop2, binaryop, 0, 26, 0, 32,
                                   BINARYOP_MUL, cnum3, cnum4);

    testsupport_parser_node_create(bop, binaryop, 0, 1, 0, 32,
                                   BINARYOP_ADD, bop1, bop2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_operator_precedence_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(a * 32) & b + 15 ^ 2 | 3");
    front_testdriver_new_ast_main_source(&s);

    // a * 32
    struct ast_node *id_a = testsupport_parser_identifier_create(0, 1, 0, 1);
    testsupport_parser_constant_create(cnum_32, 0, 5, 0, 6, integer, 32);
    testsupport_parser_node_create(bop1, binaryop, 0, 1, 0, 6,
                                   BINARYOP_MUL, id_a, cnum_32);

    // b + 15
    struct ast_node *id_b = testsupport_parser_identifier_create(0, 11, 0, 11);
    testsupport_parser_constant_create(cnum_15, 0, 15, 0, 16, integer, 15);
    testsupport_parser_node_create(bop2, binaryop, 0, 11, 0, 16,
                                   BINARYOP_ADD, id_b, cnum_15);

    // (a * 32) & b + 15
    testsupport_parser_node_create(bop3, binaryop, 0, 1, 0, 16,
                                   BINARYOP_BITWISE_AND, bop1, bop2);


    // (a * 32) & b + 15 ^ 2
    testsupport_parser_constant_create(cnum_2, 0, 20, 0, 20, integer, 2);
    testsupport_parser_node_create(bop4, binaryop, 0, 1, 0, 20,
                                   BINARYOP_BITWISE_XOR, bop3, cnum_2);


    // (a * 32) & b + 15 ^ 2 | 3
    testsupport_parser_constant_create(cnum_3, 0, 24, 0, 24, integer, 3);
    testsupport_parser_node_create(bop5, binaryop, 0, 1, 0, 24,
                                   BINARYOP_BITWISE_OR, bop4, cnum_3);

    ck_test_parse_as(n, expression, "binary operator", bop5);

    ast_node_destroy(n);
    ast_node_destroy(bop5);
}END_TEST

START_TEST(test_acc_operator_precedence_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a.foo[13]");
    front_testdriver_new_ast_main_source(&s);

    // a.foo
    struct ast_node *id_a = testsupport_parser_identifier_create(0, 0, 0, 0);
    struct ast_node *id_foo = testsupport_parser_identifier_create(0, 2, 0, 4);
    testsupport_parser_node_create(bop1, binaryop, 0, 0, 0, 4,
                                   BINARYOP_MEMBER_ACCESS, id_a, id_foo);
    // a.foo[13]
    testsupport_parser_constant_create(cnum_13, 0, 6, 0, 7, integer, 13);
    testsupport_parser_node_create(arr_ref, binaryop, 0, 0, 0, 8,
                                   BINARYOP_INDEX_ACCESS, bop1, cnum_13);

    ck_test_parse_as(n, expression, "binary operator", arr_ref);

    ast_node_destroy(n);
    ast_node_destroy(arr_ref);
}END_TEST

START_TEST(test_acc_operator_precedence_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "boo(a.foo[13] - 10).member");
    front_testdriver_new_ast_main_source(&s);

    // a.foo
    struct ast_node *id_a = testsupport_parser_identifier_create(0, 4, 0, 4);
    struct ast_node *id_foo = testsupport_parser_identifier_create(0, 6, 0, 8);
    testsupport_parser_node_create(bop1, binaryop, 0, 4, 0, 8,
                                   BINARYOP_MEMBER_ACCESS, id_a, id_foo);
    // a.foo[13]
    testsupport_parser_constant_create(cnum_13, 0, 10, 0, 11, integer, 13);
    testsupport_parser_node_create(arr_ref, binaryop, 0, 4, 0, 12,
                                   BINARYOP_INDEX_ACCESS, bop1, cnum_13);

    // a.foo[13] - 10
    testsupport_parser_constant_create(cnum_10, 0, 16, 0, 17, integer, 10);
    testsupport_parser_node_create(bop2, binaryop, 0, 4, 0, 17,
                                   BINARYOP_SUB, arr_ref, cnum_10);

    // boo(a.foo[13] - 10)
    struct ast_node *id_boo = testsupport_parser_identifier_create(0, 0, 0, 2);
    testsupport_parser_node_create(fc, fncall, 0, 0, 0, 18,
                                   id_boo,
                                   bop2,
                                   NULL);

    // boo(a.foo[13] - 10).member
    struct ast_node *id_member = testsupport_parser_identifier_create(0, 20, 0, 25);
    testsupport_parser_node_create(bop3, binaryop, 0, 0, 0, 25,
                                   BINARYOP_MEMBER_ACCESS, fc, id_member);

    ck_test_parse_as(n, expression, "binary operator", bop3);

    ast_node_destroy(n);
    ast_node_destroy(bop3);
}END_TEST

START_TEST(test_acc_subtract_negative_constant_literal) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "214 - -5651");
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_constant_create(cnum1, 0, 0, 0, 2, integer, 214);
    testsupport_parser_constant_create(cnum2, 0, 6, 0, 10, integer, -5651);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 10,
                                   BINARYOP_SUB,
                                   cnum1, cnum2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_assign_negative_constant_literal) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a = -5651");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id_a = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_constant_create(cnum2, 0, 4, 0, 8, integer, -5651);
    testsupport_parser_node_create(bop, binaryop, 0, 0, 0, 8,
                                   BINARYOP_ASSIGN,
                                   id_a, cnum2);

    ck_test_parse_as(n, expression, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

Suite *parser_operators_suite_create(void)
{
    Suite *s = suite_create("parser_operators");

    TCase *sbop = tcase_create("parser_operators_simple_binary");
    tcase_add_checked_fixture(sbop, setup_front_tests, teardown_front_tests);
    tcase_add_test(sbop, test_acc_addition);
    tcase_add_test(sbop, test_acc_multi);
    tcase_add_test(sbop, test_acc_sub);
    tcase_add_test(sbop, test_acc_div);
    tcase_add_test(sbop, test_acc_assignment);

    TCase *cbop = tcase_create("parser_operators_complex_binary");
    tcase_add_checked_fixture(cbop, setup_front_tests, teardown_front_tests);
    tcase_add_test(cbop, test_acc_complex_binary_op_1);
    tcase_add_test(cbop, test_acc_complex_binary_op_2);
    tcase_add_test(cbop, test_acc_complex_binary_op_3);
    tcase_add_test(cbop, test_acc_complex_binary_op_4);

    TCase *suop = tcase_create("parser_operators_simple_unary");
    tcase_add_checked_fixture(suop, setup_front_tests, teardown_front_tests);
    tcase_add_test(suop, test_acc_minus);
    tcase_add_test(suop, test_acc_plus);

    TCase *op_predence = tcase_create("parser_operator_precedence");
    tcase_add_checked_fixture(op_predence, setup_front_tests, teardown_front_tests);
    tcase_add_test(op_predence, test_acc_operator_precedence_1);
    tcase_add_test(op_predence, test_acc_operator_precedence_2);
    tcase_add_test(op_predence, test_acc_operator_precedence_3);
    tcase_add_test(op_predence, test_acc_subtract_negative_constant_literal);
    tcase_add_test(op_predence, test_acc_assign_negative_constant_literal);

    suite_add_tcase(s, sbop);
    suite_add_tcase(s, cbop);
    suite_add_tcase(s, suop);
    suite_add_tcase(s, op_predence);
    return s;
}
