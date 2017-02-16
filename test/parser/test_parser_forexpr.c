#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rfbase/string/core.h>

#include <parser/parser.h>
#include "../../src/parser/recursive_descent/forexpr.h"
#include "../../src/parser/recursive_descent/block.h"
#include <ast/forexpr.h>
#include <ast/iterable.h>
#include <ast/function.h>
#include <ast/operators.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>
#include <ast/constants.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_simple_forexpr_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in array {\n"
        "    do_sth(a)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 4, 0, 4);
    testsupport_parser_iterable_identifier_create(id2, 0, 9, 0, 13);
    testsupport_parser_block_create(bnode, 0, 15, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        1, 4, 1, 9
    );

    struct ast_node *id3 = testsupport_parser_identifier_create(1, 11, 1, 11);
    testsupport_parser_node_create(fc, fncall, 1, 4, 1, 12, fn_name, id3, NULL);
    ast_node_add_child(bnode, fc);

    testsupport_parser_node_create(fexpr, forexpr, 0, 0, 2, 0, id1, id2, bnode);
    ck_test_parse_as(n, forexpr, "for_expression", fexpr);

    ast_node_destroy(n);
    ast_node_destroy(fexpr);
}END_TEST

START_TEST(test_acc_simple_forexpr_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    b:u64\n"
        "    for a in array {\n"
        "        b = b + a\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_block_create(bnode, 0, 0, 5, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_u64, 1, 6, 1, 8);
    testsupport_parser_node_create(type1, typeleaf, 1, 4, 1, 8, id_b, id_u64);
    testsupport_parser_node_create(vardecl, vardecl, 1, 4, 1, 8, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id_a = testsupport_parser_identifier_create(2, 8, 2, 8);
    testsupport_parser_iterable_identifier_create(id_arr, 2, 13, 2, 17);
    testsupport_parser_block_create(forblock, 2, 19, 4, 4);

    struct ast_node *id_b2 = testsupport_parser_identifier_create(3, 8, 3, 8);
    struct ast_node *id_b3 = testsupport_parser_identifier_create(3, 12, 3, 12);
    struct ast_node *id_a2 = testsupport_parser_identifier_create(3, 16, 3, 16);
    testsupport_parser_node_create(
        addition, binaryop, 3, 12, 3, 16,
        BINARYOP_ADD,
        id_b3, id_a2
    );
    testsupport_parser_node_create(
        assignment, binaryop, 3, 8, 3, 16,
        BINARYOP_ASSIGN,
        id_b2, addition
    );
    ast_node_add_child(forblock, assignment);
    testsupport_parser_node_create(fexpr, forexpr, 2, 4, 4, 4, id_a, id_arr, forblock);

    ast_node_add_child(bnode, fexpr);

    ck_test_parse_as(n, block, "block with for_expression", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_range_forexpr_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    b:u64\n"
        "    for a in 0:25 {\n"
        "        b = b + a\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_block_create(bnode, 0, 0, 5, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_u64, 1, 6, 1, 8);
    testsupport_parser_node_create(type1, typeleaf, 1, 4, 1, 8, id_b, id_u64);
    testsupport_parser_node_create(vardecl, vardecl, 1, 4, 1, 8, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id_a = testsupport_parser_identifier_create(2, 8, 2, 8);
    testsupport_parser_constant_create(const0, 2, 13, 2, 13, integer, 0);
    testsupport_parser_constant_create(const25, 2, 15, 2, 16, integer, 25);
    testsupport_parser_iterable_range_create(niterable, const0, NULL, const25);
    testsupport_parser_block_create(forblock, 2, 18, 4, 4);

    struct ast_node *id_b2 = testsupport_parser_identifier_create(3, 8, 3, 8);
    struct ast_node *id_b3 = testsupport_parser_identifier_create(3, 12, 3, 12);
    struct ast_node *id_a2 = testsupport_parser_identifier_create(3, 16, 3, 16);
    testsupport_parser_node_create(
        addition, binaryop, 3, 12, 3, 16,
        BINARYOP_ADD,
        id_b3, id_a2
    );
    testsupport_parser_node_create(
        assignment, binaryop, 3, 8, 3, 16,
        BINARYOP_ASSIGN,
        id_b2, addition
    );
    ast_node_add_child(forblock, assignment);
    testsupport_parser_node_create(fexpr, forexpr, 2, 4, 4, 4, id_a, niterable, forblock);

    ast_node_add_child(bnode, fexpr);

    ck_test_parse_as(n, block, "block with for_expression", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_range_forexpr_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    b:u64\n"
        "    for a in 0:2:5 {\n"
        "        b = b + a\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_block_create(bnode, 0, 0, 5, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_u64, 1, 6, 1, 8);
    testsupport_parser_node_create(type1, typeleaf, 1, 4, 1, 8, id_b, id_u64);
    testsupport_parser_node_create(vardecl, vardecl, 1, 4, 1, 8, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id_a = testsupport_parser_identifier_create(2, 8, 2, 8);
    testsupport_parser_constant_create(const0, 2, 13, 2, 13, integer, 0);
    testsupport_parser_constant_create(const2, 2, 15, 2, 15, integer, 2);
    testsupport_parser_constant_create(const5, 2, 17, 2, 17, integer, 5);
    testsupport_parser_iterable_range_create(niterable, const0, const2, const5);
    testsupport_parser_block_create(forblock, 2, 19, 4, 4);

    struct ast_node *id_b2 = testsupport_parser_identifier_create(3, 8, 3, 8);
    struct ast_node *id_b3 = testsupport_parser_identifier_create(3, 12, 3, 12);
    struct ast_node *id_a2 = testsupport_parser_identifier_create(3, 16, 3, 16);
    testsupport_parser_node_create(
        addition, binaryop, 3, 12, 3, 16,
        BINARYOP_ADD,
        id_b3, id_a2
    );
    testsupport_parser_node_create(
        assignment, binaryop, 3, 8, 3, 16,
        BINARYOP_ASSIGN,
        id_b2, addition
    );
    ast_node_add_child(forblock, assignment);
    testsupport_parser_node_create(fexpr, forexpr, 2, 4, 4, 4, id_a, niterable, forblock);

    ast_node_add_child(bnode, fexpr);

    ck_test_parse_as(n, block, "block with for_expression", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_range_forexpr_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    b:u64\n"
        "    step:i64\n"
        "    for a in 0:step:5 {\n"
        "        b = b + a\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_block_create(bnode, 0, 0, 6, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_u64, 1, 6, 1, 8);
    testsupport_parser_node_create(type1, typeleaf, 1, 4, 1, 8, id_b, id_u64);
    testsupport_parser_node_create(vardecl1, vardecl, 1, 4, 1, 8, type1);
    struct ast_node *id_step = testsupport_parser_identifier_create(2, 4, 2, 7);
    testsupport_parser_xidentifier_create_simple(id_i64, 2, 9, 2, 11);
    testsupport_parser_node_create(type2, typeleaf, 2, 4, 2, 11, id_step, id_i64);
    testsupport_parser_node_create(vardecl2, vardecl, 2, 4, 2, 11, type2);

    ast_node_add_child(bnode, vardecl1);
    ast_node_add_child(bnode, vardecl2);

    struct ast_node *id_a = testsupport_parser_identifier_create(3, 8, 3, 8);
    testsupport_parser_constant_create(const0, 3, 13, 3, 13, integer, 0);
    struct ast_node *id_step2 = testsupport_parser_identifier_create(3, 15, 3, 18);
    testsupport_parser_constant_create(const5, 3, 20, 3, 20, integer, 5);
    testsupport_parser_iterable_range_create(niterable, const0, id_step2, const5);
    testsupport_parser_block_create(forblock, 3, 22, 5, 4);

    struct ast_node *id_b2 = testsupport_parser_identifier_create(4, 8, 4, 8);
    struct ast_node *id_b3 = testsupport_parser_identifier_create(4, 12, 4, 12);
    struct ast_node *id_a2 = testsupport_parser_identifier_create(4, 16, 4, 16);
    testsupport_parser_node_create(
        addition, binaryop, 4, 12, 4, 16,
        BINARYOP_ADD,
        id_b3, id_a2
    );
    testsupport_parser_node_create(
        assignment, binaryop, 4, 8, 4, 16,
        BINARYOP_ASSIGN,
        id_b2, addition
    );
    ast_node_add_child(forblock, assignment);
    testsupport_parser_node_create(fexpr, forexpr, 3, 4, 5, 4, id_a, niterable, forblock);

    ast_node_add_child(bnode, fexpr);

    ck_test_parse_as(n, block, "block with for_expression", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_range_forexpr_4) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    b:u64\n"
        "    start:i64\n"
        "    end:i64\n"
        "    for a in start:b:end {\n"
        "        b = b + 1\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    testsupport_parser_block_create(bnode, 0, 0, 7, 0);
    struct ast_node *id_b = testsupport_parser_identifier_create(1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_u64, 1, 6, 1, 8);
    testsupport_parser_node_create(type1, typeleaf, 1, 4, 1, 8, id_b, id_u64);
    testsupport_parser_node_create(vardecl1, vardecl, 1, 4, 1, 8, type1);
    struct ast_node *id_start = testsupport_parser_identifier_create(2, 4, 2, 8);
    testsupport_parser_xidentifier_create_simple(id_i64, 2, 10, 2, 12);
    testsupport_parser_node_create(type2, typeleaf, 2, 4, 2, 12, id_start, id_i64);
    testsupport_parser_node_create(vardecl2, vardecl, 2, 4, 2, 12, type2);
    struct ast_node *id_end = testsupport_parser_identifier_create(3, 4, 3, 6);
    testsupport_parser_xidentifier_create_simple(id2_i64, 3, 8, 3, 10);
    testsupport_parser_node_create(type3, typeleaf, 3, 4, 3, 10, id_end, id2_i64);
    testsupport_parser_node_create(vardecl3, vardecl, 3, 4, 3, 10, type3);

    ast_node_add_child(bnode, vardecl1);
    ast_node_add_child(bnode, vardecl2);
    ast_node_add_child(bnode, vardecl3);

    struct ast_node *id_a = testsupport_parser_identifier_create(4, 8, 4, 8);
    struct ast_node *id_start2 = testsupport_parser_identifier_create(4, 13, 4, 17);
    struct ast_node *id_b4 = testsupport_parser_identifier_create(4, 19, 4, 19);
    struct ast_node *id_end2 = testsupport_parser_identifier_create(4, 21, 4, 23);
    testsupport_parser_iterable_range_create(niterable, id_start2, id_b4, id_end2);
    testsupport_parser_block_create(forblock, 4, 25, 6, 4);

    struct ast_node *id_b2 = testsupport_parser_identifier_create(5, 8, 5, 8);
    struct ast_node *id_b3 = testsupport_parser_identifier_create(5, 12, 5, 12);
    testsupport_parser_constant_create(const1, 5, 16, 5, 16, integer, 1);
    testsupport_parser_node_create(
        addition, binaryop, 5, 12, 5, 16,
        BINARYOP_ADD,
        id_b3, const1
    );
    testsupport_parser_node_create(
        assignment, binaryop, 5, 8, 5, 16,
        BINARYOP_ASSIGN,
        id_b2, addition
    );
    ast_node_add_child(forblock, assignment);
    testsupport_parser_node_create(fexpr, forexpr, 4, 4, 6, 4, id_a, niterable, forblock);

    ast_node_add_child(bnode, fexpr);

    ck_test_parse_as(n, block, "block with for_expression", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_forexpr_errors_1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for "
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the loop variable after 'for'",
            0, 0),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a stuff"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected 'in' after the for loop variable",
            0, 6),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an iterable after 'in'",
            0, 6),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in array"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a block following the 'for' expression",
            0, 9),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in 1&2"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an iterable after 'in'",
            0, 9),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_6) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in 2:\"foo\""
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "An identifier or constant should follow the ':'",
            0, 11),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an iterable after 'in'",
            0, 9),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_forexpr_errors_7) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in 0:2:\"foo\""
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(forexpr);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "An identifier or constant should follow the second ':'",
            0, 13),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an iterable after 'in'",
            0, 9),
    };
    ck_assert_parser_errors(errors);
}END_TEST

Suite *parser_forexpr_suite_create(void)
{
    Suite *s = suite_create("parser_for_expression");

    TCase *fep = tcase_create("simple_for_expression_parsing");
    tcase_add_checked_fixture(fep, setup_front_tests, teardown_front_tests);
    tcase_add_test(fep, test_acc_simple_forexpr_1);
    tcase_add_test(fep, test_acc_simple_forexpr_2);

    TCase *frp = tcase_create("range_for_expression_parsing");
    tcase_add_checked_fixture(frp, setup_front_tests, teardown_front_tests);
    tcase_add_test(frp, test_acc_range_forexpr_1);
    tcase_add_test(frp, test_acc_range_forexpr_2);
    tcase_add_test(frp, test_acc_range_forexpr_3);
    tcase_add_test(frp, test_acc_range_forexpr_4);

    TCase *ferr = tcase_create("for_expression_parsing_errors");
    tcase_add_checked_fixture(ferr, setup_front_tests, teardown_front_tests);
    tcase_add_test(ferr, test_acc_forexpr_errors_1);
    tcase_add_test(ferr, test_acc_forexpr_errors_2);
    tcase_add_test(ferr, test_acc_forexpr_errors_3);
    tcase_add_test(ferr, test_acc_forexpr_errors_4);
    tcase_add_test(ferr, test_acc_forexpr_errors_5);
    tcase_add_test(ferr, test_acc_forexpr_errors_6);
    tcase_add_test(ferr, test_acc_forexpr_errors_7);

    suite_add_tcase(s, fep);
    suite_add_tcase(s, frp);
    suite_add_tcase(s, ferr);

    return s;
}
