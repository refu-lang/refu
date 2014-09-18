#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include "../../src/parser/recursive_descent/type.h"
#include <ast/ast.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typedesc_simple1) {
    struct ast_node *n;
    struct front_ctx *front;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    file = &front->file;
    ck_assert_msg(front, "Failed to assign string to file ");

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 0, 2, 0, 4);
    testsupport_parser_node_create(t1, typedesc, file, 0, 0, 0, 4, id1, id2);

    ck_test_parse_as(n, typedesc, d, "type description", t1);

    ast_node_destroy(n);
    ast_node_destroy(t1);
}END_TEST

START_TEST(test_acc_typedesc_simple2) {
    struct ast_node *n;
    struct front_ctx *front;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a : \t  i16");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    file = &front->file;
    ck_assert_msg(front, "Failed to assign string to file ");


    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 0, 7, 0, 9);
    testsupport_parser_node_create(t1, typedesc, file, 0, 0, 0, 9, id1, id2);

    ck_test_parse_as(n, typedesc, d, "type description", t1);

    ast_node_destroy(n);
    ast_node_destroy(t1);
}END_TEST

START_TEST(test_acc_typedesc_fail1) {
    struct ast_node *n;
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT("");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typedesc(d->front.parser);
    ck_assert_msg(n == NULL, "parsing type description should fail");
}END_TEST

START_TEST(test_acc_typedesc_fail2) {
    struct ast_node *n;
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT(" : ,");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typedesc(d->front.parser);
    ck_assert_msg(n == NULL, "parsing type description should fail");
}END_TEST

START_TEST(test_acc_typedesc_fail3) {
    struct ast_node *n;
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT("foo:int ,");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typedesc(d->front.parser);
    ck_assert_msg(n == NULL, "parsing type description should fail");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(&front->file,
                            "Expected a '(' or identifier after ','",
                            0, 8)
    };
    ck_assert_parser_errors(front->info, errors);
}END_TEST

#if 0
START_TEST(test_acc_typedesc_prod1) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16, b:i32");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_p(f);



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
    sp = parser_file_p(f);



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
    sp = parser_file_p(f);



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
#endif
Suite *parser_typedesc_suite_create(void)
{
    Suite *s = suite_create("parser_type_description");

    TCase *simple = tcase_create("parser_type_description_simple");
    tcase_add_checked_fixture(simple, setup_front_tests, teardown_front_tests);
    tcase_add_test(simple, test_acc_typedesc_simple1);
    tcase_add_test(simple, test_acc_typedesc_simple2);

    tcase_add_test(simple, test_acc_typedesc_fail1);
    tcase_add_test(simple, test_acc_typedesc_fail2);
    tcase_add_test(simple, test_acc_typedesc_fail3);

#if 0
    tcase_add_test(simple, test_acc_typedesc_prod1);
    tcase_add_test(simple, test_acc_typedesc_prod2);

    TCase *complex = tcase_create("parser_type_description_complex");
    tcase_add_checked_fixture(complex,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(complex, test_acc_typedesc_sum_associativity);
#endif
    suite_add_tcase(s, simple);
#if 0
    suite_add_tcase(s, complex);
#endif
    return s;
}
