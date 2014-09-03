#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/type.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typedesc_simple1) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);

    struct ast_node *id_1 = ast_identifier_create(f, sp, sp);
    struct ast_node *id_2 = ast_xidentifier_create(
        f, sp + 2, sp + 4, ast_identifier_create(f, sp + 2, sp + 4),
        false, false
    );
    struct ast_node *type = ast_typedesc_create(f, sp, sp + 4, id_1, id_2);

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_parsed_node(n, d, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 4);
    check_ast_match(n, type);

    ast_node_destroy(n);
    ast_node_destroy(type);
}END_TEST

START_TEST(test_acc_typedesc_simple2) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a : \t  i16");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);


    struct ast_node *id_1 = ast_identifier_create(f, sp, sp);
    struct ast_node *id_2 = ast_xidentifier_create(
        f, sp + 7, sp + 9, ast_identifier_create(f, sp + 7, sp + 9),
        false, false
    );
    struct ast_node *type = ast_typedesc_create(f, sp, sp + 9, id_1, id_2);


    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_parsed_node(n, d, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 9);
    check_ast_match(n, type);

    ast_node_destroy(n);
    ast_node_destroy(type);
}END_TEST

START_TEST(test_acc_typedesc_fail1) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_msg(n == NULL, "parsing type description should fail");
    ck_assert_driver_offset_eq(d, 0, 0, 0);
    ck_assert_rf_str_eq_cstr(parser_file_str(f), "");
}END_TEST

START_TEST(test_acc_typedesc_fail2) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT(" : ,");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_msg(n == NULL, "parsing type description should fail");
    ck_assert_driver_offset_eq(d, 0, 0, 0);
    ck_assert_rf_str_eq_cstr(parser_file_str(f), " : ,");
}END_TEST

START_TEST(test_acc_typedesc_fail3) {
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("foo:int ,");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_msg(n == NULL, "parsing type description should fail");
    ck_assert_parser_error(
        d,
        "test_file:0:9 error: Expected an identifier or '(' after a "
        "type operator\n"
        "foo:int ,\n"
        "         ^\n"
    );
    ck_assert_driver_offset_eq(d, 0, 0, 0);
    ck_assert_rf_str_eq_cstr(parser_file_str(f), "foo:int ,");
}END_TEST



START_TEST(test_acc_typedesc_prod1) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16, b:i32");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);



    struct ast_node *id1 = ast_identifier_create(f, sp, sp);
    struct ast_node *id2 = ast_xidentifier_create(
        f, sp + 2, sp + 4, ast_identifier_create(f, sp + 2, sp + 4),
        false, false
    );
    struct ast_node *type1 = ast_typedesc_create(f, sp, sp + 4, id1, id2);
    struct ast_node *id3 = ast_identifier_create(f, sp + 7, sp + 7);
    struct ast_node *id4 = ast_xidentifier_create(
        f, sp + 9, sp + 11, ast_identifier_create(f, sp + 9, sp + 11),
        false, false
    );
    struct ast_node *type2 = ast_typedesc_create(f, sp + 7, sp + 11, id3, id4);
    struct ast_node *op = ast_typeop_create(f, sp, sp + 11,
                                            TYPEOP_PRODUCT,
                                            type1, type2);


    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_parsed_node(n, d, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 11);
    check_ast_match(n, op);

    ast_node_destroy(n);
    ast_node_destroy(op);
}END_TEST

START_TEST(test_acc_typedesc_prod2) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16, b:i32, "
                                                           "c:f64");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);



    struct ast_node *id1 = ast_identifier_create(f, sp, sp);
    struct ast_node *id2 = ast_xidentifier_create(
        f, sp + 2, sp + 4, ast_identifier_create(f, sp + 2, sp + 4),
        false, false
    );
    struct ast_node *type1 = ast_typedesc_create(f, sp, sp + 4, id1, id2);
    struct ast_node *id3 = ast_identifier_create(f, sp + 7, sp + 7);
    struct ast_node *id4 = ast_xidentifier_create(
        f, sp + 9, sp + 11, ast_identifier_create(f, sp + 9, sp + 11),
        false, false
    );
    struct ast_node *type2 = ast_typedesc_create(f, sp + 7, sp + 11, id3, id4);
    struct ast_node *op1 = ast_typeop_create(f, sp, sp + 11,
                                            TYPEOP_PRODUCT,
                                            type1, type2);
    struct ast_node *id5 = ast_identifier_create(f, sp + 14, sp + 14);
    struct ast_node *id6 = ast_xidentifier_create(
        f, sp + 16, sp + 18, ast_identifier_create(f, sp + 16, sp + 18),
        false, false
    );
    struct ast_node *type3 = ast_typedesc_create(f, sp + 14, sp + 18, id5, id6);
    struct ast_node *op2 = ast_typeop_create(f, sp, sp + 18,
                                            TYPEOP_PRODUCT,
                                            op1, type3);
    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_parsed_node(n, d, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 18);
    check_ast_match(n, op2);

    ast_node_destroy(n);
    ast_node_destroy(op2);
}END_TEST


// test a simple case of right associativity of the sum type operator
START_TEST(test_acc_typedesc_sum_associativity) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16, b:i32 | "
                                                           "c:f64, d:f32");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);



    struct ast_node *id1 = ast_identifier_create(f, sp, sp);
    struct ast_node *id2 = ast_xidentifier_create(
        f, sp + 2, sp + 4, ast_identifier_create(f, sp + 2, sp + 4),
        false, false
    );
    struct ast_node *type1 = ast_typedesc_create(f, sp, sp + 4, id1, id2);
    struct ast_node *id3 = ast_identifier_create(f, sp + 7, sp + 7);
    struct ast_node *id4 = ast_xidentifier_create(
        f, sp + 9, sp + 11, ast_identifier_create(f, sp + 9, sp + 11),
        false, false
    );
    struct ast_node *type2 = ast_typedesc_create(f, sp + 7, sp + 11, id3, id4);
    struct ast_node *op1 = ast_typeop_create(f, sp, sp + 11,
                                             TYPEOP_PRODUCT,
                                             type1, type2);

    struct ast_node *id5 = ast_identifier_create(f, sp + 15, sp + 15);
    struct ast_node *id6 = ast_xidentifier_create(
        f, sp + 17, sp + 19, ast_identifier_create(f, sp + 17, sp + 19),
        false, false
    );
    struct ast_node *type3 = ast_typedesc_create(f, sp + 15, sp + 19, id5, id6);

    struct ast_node *id7 = ast_identifier_create(f, sp + 22, sp + 22);
    struct ast_node *id8 = ast_xidentifier_create(
        f, sp + 24, sp + 26, ast_identifier_create(f, sp + 24, sp + 26),
        false, false
    );
    struct ast_node *type4 = ast_typedesc_create(f, sp + 22, sp + 26, id7, id8);
    struct ast_node *op2 = ast_typeop_create(f, sp + 15, sp + 26,
                                             TYPEOP_PRODUCT,
                                             type3, type4);

    struct ast_node *op_sum = ast_typeop_create(f, sp, sp + 26,
                                                TYPEOP_SUM,
                                                op1, op2);

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_parsed_node(n, d, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 26);
    check_ast_match(n, op_sum);

    ast_node_destroy(n);
    ast_node_destroy(op_sum);
}END_TEST

Suite *parser_typedesc_suite_create(void)
{
    Suite *s = suite_create("parser_type_description");

    TCase *simple = tcase_create("parser_type_description_simple");
    tcase_add_checked_fixture(simple, setup_parser_tests, teardown_parser_tests);
    tcase_add_test(simple, test_acc_typedesc_simple1);
    tcase_add_test(simple, test_acc_typedesc_simple2);
    tcase_add_test(simple, test_acc_typedesc_fail1);
    tcase_add_test(simple, test_acc_typedesc_fail2);
    tcase_add_test(simple, test_acc_typedesc_fail3);

    tcase_add_test(simple, test_acc_typedesc_prod1);
    tcase_add_test(simple, test_acc_typedesc_prod2);

    TCase *complex = tcase_create("parser_type_description_complex");
    tcase_add_checked_fixture(complex,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(complex, test_acc_typedesc_sum_associativity);

    suite_add_tcase(s, simple);
    suite_add_tcase(s, complex);
    return s;
}
