#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
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
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a + b");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

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
    file = d->front.file;

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
    file = d->front.file;

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
    file = d->front.file;

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

START_TEST(test_acc_assignment) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a = 5 + 2.149");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       0, 4, 0, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 8, 0, 12, float, 2.149);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 4, 0, 12,
                                   BINARYOP_ADD,
                                   cnum1, cnum2);

    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 12,
                                   BINARYOP_ASSIGN,
                                   id1, bop1);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a + 12.232 * 5 - foo");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       0, 4, 0, 9, float, 12.232);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 13, 0, 13, integer, 5);

    testsupport_parser_node_create(bop1, binaryop, file, 0, 4, 0, 13,
                                   BINARYOP_MUL,
                                   cnum1, cnum2);
    testsupport_parser_node_create(bop2, binaryop, file, 0, 0, 0, 13,
                                   BINARYOP_ADD,
                                   id1, bop1);

    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 17, 0, 19);
    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 19,
                                   BINARYOP_SUB,
                                   bop2, id2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_2) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "\"start\" + 1.0e-10 - (a + dosth(42, foo))");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;


    testsupport_parser_string_literal_create(literal, file, 0, 0, 0, 6);
    testsupport_parser_constant_create(cnum1, file,
                                       0, 10, 0, 16, float, 1.0e-10);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 0, 0, 16,
                                   BINARYOP_ADD,
                                   literal, cnum1);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 21, 0, 21);


    struct ast_node *fn_name = testsupport_parser_identifier_create(file,
                                                                    0, 25, 0, 29);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 31, 0, 32, integer, 42);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 35, 0, 37);

    testsupport_parser_node_create(arg_bop, binaryop, file, 0, 31, 0, 37,
                                   BINARYOP_COMMA,
                                   cnum2, id2);
    testsupport_parser_node_create(fn, fncall, file, 0, 25, 0, 38,
                                   fn_name, arg_bop, NULL);

    testsupport_parser_node_create(bop2, binaryop, file, 0, 21, 0, 38,
                                   BINARYOP_ADD,
                                   id1, fn);

    testsupport_parser_node_create(bop, binaryop, file, 0, 0, 0, 38,
                                   BINARYOP_SUB,
                                   bop1, bop2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_3) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(foo<a, b>((25/2) * 323 + 2) + 325) * 3.14");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *fn_name = testsupport_parser_identifier_create(file,
                                                                    0, 1, 0, 3);

    testsupport_parser_xidentifier_create_simple(id1, file,
                                                 0, 5, 0, 5);
    testsupport_parser_xidentifier_create_simple(id2, file,
                                                 0, 8, 0, 8);
    testsupport_parser_node_create(gattr, genrattr, file, 0, 4, 0, 9);
    ast_node_add_child(gattr, id1);
    ast_node_add_child(gattr, id2);

    testsupport_parser_constant_create(cnum1, file,
                                       0, 12, 0, 13, integer, 25);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 15, 0, 15, integer, 2);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 12, 0, 15,
                                   BINARYOP_DIV,
                                   cnum1, cnum2);

    testsupport_parser_constant_create(cnum3, file,
                                       0, 20, 0, 22, integer, 323);
    testsupport_parser_node_create(bop2, binaryop, file, 0, 12, 0, 22,
                                   BINARYOP_MUL,
                                   bop1, cnum3);

    testsupport_parser_constant_create(cnum4, file,
                                       0, 26, 0, 26, integer, 2);
    testsupport_parser_node_create(bop3, binaryop, file, 0, 12, 0, 26,
                                   BINARYOP_ADD,
                                   bop2, cnum4);

    testsupport_parser_node_create(fn, fncall, file, 0, 1, 0, 27,
                                   fn_name, bop3, gattr);

    testsupport_parser_constant_create(cnum5, file,
                                       0, 31, 0, 33, integer, 325);
    testsupport_parser_node_create(bop4, binaryop, file, 0, 1, 0, 33,
                                   BINARYOP_ADD,
                                   fn, cnum5);

    testsupport_parser_constant_create(cnum6, file,
                                       0, 38, 0, 41, float, 3.14);
    testsupport_parser_node_create(bop, binaryop, file, 0, 1, 0, 41,
                                   BINARYOP_MUL,
                                   bop4, cnum6);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_complex_binary_op_4) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(table[56] + foo(3, b)) + 4 * 321");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 1, 0, 5);
    testsupport_parser_constant_create(cnum1, file,
                                       0, 7, 0, 8, integer, 56);
    testsupport_parser_node_create(arr, binaryop, file, 0, 1, 0, 9,
                                   BINARYOP_ARRAY_REFERENCE, id1, cnum1);

    struct ast_node *fn_name = testsupport_parser_identifier_create(file,
                                                                    0, 13, 0, 15);
    testsupport_parser_constant_create(cnum2, file,
                                       0, 17, 0, 17, integer, 3);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 20, 0, 20);
    testsupport_parser_node_create(arg_bop, binaryop, file, 0, 17, 0, 20,
                                   BINARYOP_COMMA,
                                   cnum2, id2);
    testsupport_parser_node_create(fn, fncall, file, 0, 13, 0, 21,
                                   fn_name, arg_bop, NULL);

    testsupport_parser_node_create(bop1, binaryop, file, 0, 1, 0, 21,
                                   BINARYOP_ADD, arr, fn);

    testsupport_parser_constant_create(cnum3, file,
                                       0, 26, 0, 26, integer, 4);
    testsupport_parser_constant_create(cnum4, file,
                                       0, 30, 0, 32, integer, 321);
    testsupport_parser_node_create(bop2, binaryop, file, 0, 26, 0, 32,
                                   BINARYOP_MUL, cnum3, cnum4);

    testsupport_parser_node_create(bop, binaryop, file, 0, 1, 0, 32,
                                   BINARYOP_ADD, bop1, bop2);

    ck_test_parse_as(n, expression, d, "binary operator", bop);

    ast_node_destroy(n);
    ast_node_destroy(bop);
}END_TEST

START_TEST(test_acc_operator_precedence_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "(a * 32) & b + 15 ^ 2 | 3");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    // a * 32
    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 1, 0, 1);
    testsupport_parser_constant_create(cnum_32, file,
                                       0, 5, 0, 6, integer, 32);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 1, 0, 6,
                                   BINARYOP_MUL, id_a, cnum_32);

    // b + 15
    struct ast_node *id_b = testsupport_parser_identifier_create(file,
                                                                 0, 11, 0, 11);
    testsupport_parser_constant_create(cnum_15, file,
                                       0, 15, 0, 16, integer, 15);
    testsupport_parser_node_create(bop2, binaryop, file, 0, 11, 0, 16,
                                   BINARYOP_ADD, id_b, cnum_15);

    // (a * 32) & b + 15
    testsupport_parser_node_create(bop3, binaryop, file, 0, 1, 0, 16,
                                   BINARYOP_BITWISE_AND, bop1, bop2);


    // (a * 32) & b + 15 ^ 2
    testsupport_parser_constant_create(cnum_2, file,
                                       0, 20, 0, 20, integer, 2);
    testsupport_parser_node_create(bop4, binaryop, file, 0, 1, 0, 20,
                                   BINARYOP_BITWISE_XOR, bop3, cnum_2);


    // (a * 32) & b + 15 ^ 2 | 3
    testsupport_parser_constant_create(cnum_3, file,
                                       0, 24, 0, 24, integer, 3);
    testsupport_parser_node_create(bop5, binaryop, file, 0, 1, 0, 24,
                                   BINARYOP_BITWISE_OR, bop4, cnum_3);

    ck_test_parse_as(n, expression, d, "binary operator", bop5);

    ast_node_destroy(n);
    ast_node_destroy(bop5);
}END_TEST

START_TEST(test_acc_operator_precedence_2) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a.foo[13]");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    // a.foo
    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 0, 0, 0);
    struct ast_node *id_foo = testsupport_parser_identifier_create(file,
                                                                   0, 2, 0, 4);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 0, 0, 4,
                                   BINARYOP_MEMBER_ACCESS, id_a, id_foo);
    // a.foo[13]
    testsupport_parser_constant_create(cnum_13, file,
                                       0, 6, 0, 7, integer, 13);
    testsupport_parser_node_create(arr_ref, binaryop, file, 0, 0, 0, 8,
                                   BINARYOP_ARRAY_REFERENCE, bop1, cnum_13);

    ck_test_parse_as(n, expression, d, "binary operator", arr_ref);

    ast_node_destroy(n);
    ast_node_destroy(arr_ref);
}END_TEST

START_TEST(test_acc_operator_precedence_3) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "boo(a.foo[13] - 10).member");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    // a.foo
    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 4, 0, 4);
    struct ast_node *id_foo = testsupport_parser_identifier_create(file,
                                                                   0, 6, 0, 8);
    testsupport_parser_node_create(bop1, binaryop, file, 0, 4, 0, 8,
                                   BINARYOP_MEMBER_ACCESS, id_a, id_foo);
    // a.foo[13]
    testsupport_parser_constant_create(cnum_13, file,
                                       0, 10, 0, 11, integer, 13);
    testsupport_parser_node_create(arr_ref, binaryop, file, 0, 4, 0, 12,
                                   BINARYOP_ARRAY_REFERENCE, bop1, cnum_13);

    // a.foo[13] - 10
    testsupport_parser_constant_create(cnum_10, file,
                                       0, 16, 0, 17, integer, 10);
    testsupport_parser_node_create(bop2, binaryop, file, 0, 4, 0, 17,
                                   BINARYOP_SUB, arr_ref, cnum_10);

    // boo(a.foo[13] - 10)
    struct ast_node *id_boo = testsupport_parser_identifier_create(file,
                                                                   0, 0, 0, 2);
    testsupport_parser_node_create(fc, fncall, file, 0, 0, 0, 18,
                                   id_boo,
                                   bop2,
                                   NULL);

    // boo(a.foo[13] - 10).member
    struct ast_node *id_member = testsupport_parser_identifier_create(file,
                                                                      0, 20, 0, 25);
    testsupport_parser_node_create(bop3, binaryop, file, 0, 0, 0, 25,
                                   BINARYOP_MEMBER_ACCESS, fc, id_member);

    ck_test_parse_as(n, expression, d, "binary operator", bop3);

    ast_node_destroy(n);
    ast_node_destroy(bop3);
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

    TCase *op_predence = tcase_create("parser_operator_precedence");
    tcase_add_checked_fixture(op_predence, setup_front_tests, teardown_front_tests);
    tcase_add_test(op_predence, test_acc_operator_precedence_1);
    tcase_add_test(op_predence, test_acc_operator_precedence_2);
    tcase_add_test(op_predence, test_acc_operator_precedence_3);

    suite_add_tcase(s, sbop);
    suite_add_tcase(s, cbop);
    suite_add_tcase(s, op_predence);
    return s;
}
