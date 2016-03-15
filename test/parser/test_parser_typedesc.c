#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/core.h>
#include "../../src/parser/recursive_descent/type.h"
#include <parser/parser.h>
#include <ast/ast.h>
#include <ast/type.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typedesc_simple1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);

    ck_test_parse_as(n, typedesc, "type description", t1);

    ast_node_destroy(n);
    ast_node_destroy(t1);
}END_TEST

START_TEST(test_acc_typedesc_simple2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a : \t  i16");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 7, 0, 9);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 9, id1, id2);

    ck_test_parse_as(n, typedesc, "type description", t1);

    ast_node_destroy(n);
    ast_node_destroy(t1);
}END_TEST

START_TEST(test_acc_typedesc_no_colon) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16 -> int");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 0, 9, 0, 11);
    testsupport_parser_node_create(op, typeop, 0, 0, 0, 11,
                                   TYPEOP_IMPLICATION, t1, id3);
    ck_test_parse_as(n, typedesc, "type description", op);

    ast_node_destroy(n);
    ast_node_destroy(op);
}END_TEST

START_TEST(test_acc_typedesc_fail1) {
    front_testdriver_new_ast_main_source(rf_string_empty_get());

    ck_test_fail_parse_noerr_as(typedesc);
}END_TEST

START_TEST(test_acc_typedesc_fail2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(" : ,");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_noerr_as(typedesc);
}END_TEST

START_TEST(test_acc_typedesc_fail3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("foo:int ,");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typedesc);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a '(' or identifier after ','",
            0, 8)
    };
    ck_assert_parser_errors(errors);
}END_TEST

static void test_simple_typeop(enum typeop_type op_type, char *str, int t2_start)
{
    struct ast_node *n;
    const struct RFstring s = RF_STRING_SHALLOW_INIT_CSTR(str);
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);

    struct ast_node *id3 = testsupport_parser_identifier_create(
        0, t2_start, 0, t2_start);
    testsupport_parser_xidentifier_create_simple(
        id4, 0, t2_start + 2, 0, t2_start + 4);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, t2_start, 0, t2_start + 4, id3, id4);
    testsupport_parser_node_create(op, typeop, 0, 0, 0, t2_start + 4,
                                   op_type, t1, t2);

    ck_test_parse_as(n, typedesc, "type description", op);

    ast_node_destroy(n);
    ast_node_destroy(op);
}

START_TEST(test_acc_typedesc_prod1) {
    test_simple_typeop(TYPEOP_PRODUCT, "a:i16, b:i32", 7);
}END_TEST

START_TEST(test_acc_typedesc_sum1) {
    test_simple_typeop(TYPEOP_SUM, "a:i16 | b:i32", 8);
}END_TEST

START_TEST(test_acc_typedesc_impl1) {
    test_simple_typeop(TYPEOP_IMPLICATION, "a:i16 -> b:i32", 9);
}END_TEST

START_TEST(test_acc_typedesc_prod2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:i16, b:i32, c:f64");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);
    struct ast_node *id3 = testsupport_parser_identifier_create(
        0, 7, 0, 7);
    testsupport_parser_xidentifier_create_simple(
        id4, 0, 9, 0, 11);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, 7, 0, 11, id3, id4);
    testsupport_parser_node_create(op1, typeop, 0, 0, 0, 11,
                                   TYPEOP_PRODUCT, t1, t2);
    struct ast_node *id5 = testsupport_parser_identifier_create(
        0, 14, 0, 14);
    testsupport_parser_xidentifier_create_simple(
        id6,
        0, 16, 0, 18);
    testsupport_parser_node_create(t3, typeleaf,
                                   0, 14, 0, 18, id5, id6);
    testsupport_parser_node_create(op2, typeop, 0, 0, 0, 18,
                                   TYPEOP_PRODUCT, op1, t3);

    ck_test_parse_as(n, typedesc, "type description", op2);
    ast_node_destroy(n);
    ast_node_destroy(op2);
}END_TEST

START_TEST(test_acc_typedesc_sum_associativity) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16, b:i32 | "
                                                           "c:f64, d:f32");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);
    struct ast_node *id3 = testsupport_parser_identifier_create(
        0, 7, 0, 7);
    testsupport_parser_xidentifier_create_simple(
        id4, 0, 9, 0, 11);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, 7, 0, 11, id3, id4);
    testsupport_parser_node_create(op1, typeop, 0, 0, 0, 11,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id5 = testsupport_parser_identifier_create(
        0, 15, 0, 15);
    testsupport_parser_xidentifier_create_simple(
        id6,
        0, 17, 0, 19);
    testsupport_parser_node_create(t3, typeleaf,
                                   0, 15, 0, 19, id5, id6);
    struct ast_node *id7 = testsupport_parser_identifier_create(
        0, 22, 0, 22);
    testsupport_parser_xidentifier_create_simple(
        id8,
        0, 24, 0, 26);
    testsupport_parser_node_create(t4, typeleaf,
                                   0, 22, 0, 26, id7, id8);
    testsupport_parser_node_create(op2, typeop, 0, 15, 0, 26,
                                   TYPEOP_PRODUCT, t3, t4);

    testsupport_parser_node_create(op_sum, typeop, 0, 0, 0, 26,
                                   TYPEOP_SUM, op1, op2);

    ck_test_parse_as(n, typedesc, "type description", op_sum);
    ast_node_destroy(n);
    ast_node_destroy(op_sum);
}END_TEST

START_TEST(test_acc_typedesc_sum_impl_associativity) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:i16, b:i32  -> c:f64 | d:f32");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 4, id1, id2);
    struct ast_node *id3 = testsupport_parser_identifier_create(
        0, 7, 0, 7);
    testsupport_parser_xidentifier_create_simple(
        id4, 0, 9, 0, 11);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, 7, 0, 11, id3, id4);
    testsupport_parser_node_create(op1, typeop, 0, 0, 0, 11,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id5 = testsupport_parser_identifier_create(
        0, 17, 0, 17);
    testsupport_parser_xidentifier_create_simple(
        id6,
        0, 19, 0, 21);
    testsupport_parser_node_create(t3, typeleaf,
                                   0, 17, 0, 21, id5, id6);
    struct ast_node *id7 = testsupport_parser_identifier_create(
        0, 25, 0, 25);
    testsupport_parser_xidentifier_create_simple(
        id8,
        0, 27, 0, 29);
    testsupport_parser_node_create(t4, typeleaf,
                                   0, 25, 0, 29, id7, id8);
    testsupport_parser_node_create(op2, typeop, 0, 17, 0, 29,
                                   TYPEOP_SUM, t3, t4);

    testsupport_parser_node_create(op_impl, typeop, 0, 0, 0, 29,
                                   TYPEOP_IMPLICATION, op1, op2);

    ck_test_parse_as(n, typedesc, "type description", op_impl);
    ast_node_destroy(n);
    ast_node_destroy(op_impl);
}END_TEST

START_TEST(test_acc_typedesc_complex_right) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:(i16|f32)");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id_a = testsupport_parser_identifier_create(0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id_i16, 0, 3, 0, 5);
    testsupport_parser_xidentifier_create_simple(id_f32, 0, 7, 0, 9);
    testsupport_parser_typedesc_create(op1, 0, 3, 0, 9, typeop,
                                   TYPEOP_SUM, id_i16, id_f32);
    testsupport_parser_node_create(t1, typeleaf, 0, 0, 0, 9, id_a, op1);
    
    ck_test_parse_as(n, typedesc, "type description", t1);
    ast_node_destroy(n);
    ast_node_destroy(t1);
}END_TEST

Suite *parser_typedesc_suite_create(void)
{
    Suite *s = suite_create("parser_type_description");

    TCase *simple = tcase_create("parser_type_description_simple");
    tcase_add_checked_fixture(simple, setup_front_tests, teardown_front_tests);
    tcase_add_test(simple, test_acc_typedesc_simple1);
    tcase_add_test(simple, test_acc_typedesc_simple2);
    tcase_add_test(simple, test_acc_typedesc_no_colon);

    tcase_add_test(simple, test_acc_typedesc_fail1);
    tcase_add_test(simple, test_acc_typedesc_fail2);
    tcase_add_test(simple, test_acc_typedesc_fail3);

    TCase *ops = tcase_create("parser_type_description_operators");
    tcase_add_checked_fixture(ops, setup_front_tests, teardown_front_tests);
    tcase_add_test(ops, test_acc_typedesc_prod1);
    tcase_add_test(ops, test_acc_typedesc_sum1);
    tcase_add_test(ops, test_acc_typedesc_impl1);
    tcase_add_test(ops, test_acc_typedesc_prod2);


    TCase *complex = tcase_create("parser_type_description_complex_operations");
    tcase_add_checked_fixture(complex, setup_front_tests, teardown_front_tests);
    tcase_add_test(complex, test_acc_typedesc_sum_associativity);
    tcase_add_test(complex, test_acc_typedesc_sum_impl_associativity);
    tcase_add_test(complex, test_acc_typedesc_complex_right);

    suite_add_tcase(s, simple);
    suite_add_tcase(s, ops);
    suite_add_tcase(s, complex);
    return s;
}
