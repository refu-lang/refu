#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/block.h"
#include <ast/function.h>
#include <ast/operators.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>
#include <ast/arrayref.h>
#include <ast/constant_num.h>
#include <ast/returnstmt.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_block_empty) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 1, 0);
    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_no_braces_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "\n"
        "a:i32\n"
        "a = 5 + 0.234"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 1, 0, 2, 12);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 2, 1, 4);
    testsupport_parser_node_create(type1, typedesc, file,
                                   1, 0, 1, 4, id1, id2);
    testsupport_parser_node_create(vardecl, vardecl, file,
                                   1, 0, 1, 4, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                2, 0, 2, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       2, 4, 2, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       2, 8, 2, 12, float, 0.234);
    testsupport_parser_node_create(op1, binaryop, file, 2, 4, 2, 12,
                                   BINARYOP_ADD, cnum1, cnum2);

    testsupport_parser_node_create(op2, binaryop, file, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id3, op1);
    ast_node_add_child(bnode, op2);

    ck_test_parse_as(n, block, d, "block", bnode, false);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_no_braces_2) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "var = buff[index] * 92.324"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 0, 25);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 0, 0, 2);

    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 6, 0, 9);
    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                0, 11, 0, 15);
    testsupport_parser_node_create(arr, arrayref, file, 
                                   0, 6, 0, 16, id2, id3);

    testsupport_parser_constant_create(cnum, file,
                                       0, 20, 0, 25, float, 92.324);
    testsupport_parser_node_create(op1, binaryop, file, 0, 6, 0, 25,
                                   BINARYOP_MUL, arr, cnum);

    testsupport_parser_node_create(op2, binaryop, file, 0, 0, 0, 25,
                                   BINARYOP_ASSIGN, id1, op1);
    ast_node_add_child(bnode, op2);

    ck_test_parse_as(n, block, d, "block", bnode, false);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32\n"
        "a = 5 + 0.234\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 3, 0);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 2, 1, 4);
    testsupport_parser_node_create(type1, typedesc, file,
                                   1, 0, 1, 4, id1, id2);
    testsupport_parser_node_create(vardecl, vardecl, file,
                                   1, 0, 1, 4, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                2, 0, 2, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       2, 4, 2, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       2, 8, 2, 12, float, 0.234);
    testsupport_parser_node_create(op1, binaryop, file, 2, 4, 2, 12,
                                   BINARYOP_ADD, cnum1, cnum2);

    testsupport_parser_node_create(op2, binaryop, file, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id3, op1);
    ast_node_add_child(bnode, op2);

    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_2) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "do_sth(eleos, \"str\", arr[15])\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 2, 0);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 5);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                1, 7, 1, 11);
    testsupport_parser_string_literal_create(str, file, 1, 14, 1, 18);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                1, 21, 1, 23);
    testsupport_parser_constant_create(cnum1, file,
                                       1, 25, 1, 26, integer, 15);
    testsupport_parser_node_create(arr, arrayref, file,
                                   1, 21, 1, 27, id3, cnum1);
    
    testsupport_parser_node_create(fn, fncall, file, 1, 0, 1, 28, id1, NULL);
    ast_node_add_child(fn, id2);
    ast_node_add_child(fn, str);
    ast_node_add_child(fn, arr);

    ast_node_add_child(bnode, fn);
    
    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_value_without_return) {
    //TODO: This actually tests that the value is NULL. Should not be like that
    // but remains to be implemented
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32\n"
        "a = 5 + 0.234\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 3, 0);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 2, 1, 4);
    testsupport_parser_node_create(type1, typedesc, file,
                                   1, 0, 1, 4, id1, id2);
    testsupport_parser_node_create(vardecl, vardecl, file,
                                   1, 0, 1, 4, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                2, 0, 2, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       2, 4, 2, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       2, 8, 2, 12, float, 0.234);
    testsupport_parser_node_create(op1, binaryop, file, 2, 4, 2, 12,
                                   BINARYOP_ADD, cnum1, cnum2);

    testsupport_parser_node_create(op2, binaryop, file, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id3, op1);
    ast_node_add_child(bnode, op2);

    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_value_with_return) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32\n"
        "a = 5 + 0.234\n"
        "return a * 2\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 4, 0);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 2, 1, 4);
    testsupport_parser_node_create(type1, typedesc, file,
                                   1, 0, 1, 4, id1, id2);
    testsupport_parser_node_create(vardecl, vardecl, file,
                                   1, 0, 1, 4, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                2, 0, 2, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       2, 4, 2, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       2, 8, 2, 12, float, 0.234);
    testsupport_parser_node_create(op1, binaryop, file, 2, 4, 2, 12,
                                   BINARYOP_ADD, cnum1, cnum2);

    testsupport_parser_node_create(op2, binaryop, file, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id3, op1);
    ast_node_add_child(bnode, op2);

    struct ast_node *id4 = testsupport_parser_identifier_create(file,
                                                                3, 7, 3, 7);
    testsupport_parser_constant_create(cnum3, file,
                                       3, 11, 3, 11, integer, 2);
    testsupport_parser_node_create(op3, binaryop, file, 3, 7, 3, 11,
                                   BINARYOP_MUL, id4, cnum3);
    testsupport_parser_node_create(ret, returnstmt, file, 3, 0, 3, 11, op3);
    ast_block_valueexpr_set(bnode, ret);
    ast_node_add_child(bnode, ret);

    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

Suite *parser_block_suite_create(void)
{
    Suite *s = suite_create("parser_block");

    TCase *fp = tcase_create("parser_block_parsing");
    tcase_add_checked_fixture(fp, setup_front_tests, teardown_front_tests);
    tcase_add_test(fp, test_acc_block_empty);
    tcase_add_test(fp, test_acc_block_no_braces_1);
    tcase_add_test(fp, test_acc_block_no_braces_2);
    tcase_add_test(fp, test_acc_block_1);
    tcase_add_test(fp, test_acc_block_2);
    tcase_add_test(fp, test_acc_block_value_without_return);
    tcase_add_test(fp, test_acc_block_value_with_return);

    suite_add_tcase(s, fp);

    return s;
}
