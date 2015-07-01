#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/ifexpr.h"
#include <ast/ifexpr.h>
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

START_TEST(test_acc_ifexpr_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a == 42 {\n"
        "    do_sth()\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 3, 0, 3);
    testsupport_parser_constant_create(cnum, 0, 8, 0, 9, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, 0, 3, 0, 9,
                                   BINARYOP_CMP_EQ, id1, cnum);


    testsupport_parser_block_create(bnode, 0, 11, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        1, 4, 1, 9);

    testsupport_parser_node_create(fc, fncall, 1, 4, 1, 11, fn_name, NULL, NULL);
    ast_node_add_child(bnode, fc);

    testsupport_parser_node_create(cond, condbranch, 0, 3, 2, 0,
                                   cmp_exp, bnode);

    testsupport_parser_node_create(ifx, ifexpr, 0, 0, 2, 0, cond, NULL);

    ck_test_parse_as(n, ifexpr, "if_expression", ifx, TOKEN_KW_IF);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST

START_TEST(test_acc_ifexpr_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a == 42 {\n"
        "    do_sth()\n"
        "} else { \n"
        "    55 + 2.31\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 3, 0, 3);
    testsupport_parser_constant_create(cnum1, 0, 8, 0, 9, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, 0, 3, 0, 9,
                                   BINARYOP_CMP_EQ, id1, cnum1);


    testsupport_parser_block_create(bnode1, 0, 11, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        1, 4, 1, 9);

    testsupport_parser_node_create(fc, fncall, 1, 4, 1, 11, fn_name, NULL, NULL);
    ast_node_add_child(bnode1, fc);

    testsupport_parser_node_create(cond1, condbranch, 0, 3, 2, 0,
                                   cmp_exp, bnode1);


    testsupport_parser_block_create(bnode2, 2, 7, 4, 0);
    testsupport_parser_constant_create(cnum2, 3, 4, 3, 5, integer, 55);
    testsupport_parser_constant_create(cnum3, 3, 9, 3, 12, float, 2.31);
    testsupport_parser_node_create(op1, binaryop, 3, 4, 3, 12,
                                   BINARYOP_ADD, cnum2, cnum3);
    ast_node_add_child(bnode2, op1);



    testsupport_parser_node_create(ifx, ifexpr, 0, 0, 4, 0, cond1, bnode2);

    ck_test_parse_as(n, ifexpr, "if_expression", ifx, TOKEN_KW_IF);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST

START_TEST(test_acc_ifexpr_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a == 42 {\n"
        "    do_sth()\n"
        "} elif (a == 50 && is_good()) {\n"
        "    \"foo\"\n"
        "} else { \n"
        "    55 + 2.31\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 3, 0, 3);
    testsupport_parser_constant_create(cnum1, 0, 8, 0, 9, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, 0, 3, 0, 9,
                                   BINARYOP_CMP_EQ, id1, cnum1);


    testsupport_parser_block_create(bnode1, 0, 11, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        1, 4, 1, 9);

    testsupport_parser_node_create(fc, fncall, 1, 4, 1, 11, fn_name, NULL, NULL);
    ast_node_add_child(bnode1, fc);

    testsupport_parser_node_create(cond1, condbranch, 0, 3, 2, 0,
                                   cmp_exp, bnode1);



    struct ast_node *id2 = testsupport_parser_identifier_create(2, 8, 2, 8);
    testsupport_parser_constant_create(cnum2, 2, 13, 2, 14, integer, 50);
    testsupport_parser_node_create(op1, binaryop, 2, 8, 2, 14,
                                   BINARYOP_CMP_EQ, id2, cnum2);
    struct ast_node *fn_name2 = testsupport_parser_identifier_create(
        2, 19, 2, 25);
    testsupport_parser_node_create(fc2, fncall, 2, 19, 2, 27,
                                   fn_name2, NULL, NULL);
    testsupport_parser_node_create(cmp_exp2, binaryop, 2, 8, 2, 27,
                                   BINARYOP_LOGIC_AND, op1, fc2);

    testsupport_parser_block_create(bnode2, 2, 30, 4, 0);
    testsupport_parser_string_literal_create(sliteral1, 3, 4, 3, 8);
    ast_node_add_child(bnode2, sliteral1);
    testsupport_parser_node_create(cond2, condbranch, 2, 8, 4, 0,
                                   cmp_exp2, bnode2);


    testsupport_parser_block_create(bnode3, 4, 7, 6, 0);
    testsupport_parser_constant_create(cnum3, 5, 4, 5, 5, integer, 55);
    testsupport_parser_constant_create(cnum4, 5, 9, 5, 12, float, 2.31);
    testsupport_parser_node_create(op3, binaryop, 5, 4, 5, 12,
                                   BINARYOP_ADD, cnum3, cnum4);
    ast_node_add_child(bnode3, op3);

    // the elif
    testsupport_parser_node_create(if2, ifexpr, 2, 2, 6, 0, cond2, bnode3);
    // the first if
    testsupport_parser_node_create(ifx, ifexpr, 0, 0, 6, 0, cond1, if2);

    ck_test_parse_as(n, ifexpr, "if_expression", ifx, TOKEN_KW_IF);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST

START_TEST(test_acc_ifexpr_4) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a == 42 {\n"
        "    do_sth()\n"
        "} elif (a == 50 && is_good()) {\n"
        "    \"foo\"\n"
        "} elif (5 < something) {\n"
        "    283.23\n"
        "} else { \n"
        "    55 + 2.31\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 3, 0, 3);
    testsupport_parser_constant_create(cnum1, 0, 8, 0, 9, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, 0, 3, 0, 9,
                                   BINARYOP_CMP_EQ, id1, cnum1);


    testsupport_parser_block_create(bnode1, 0, 11, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        1, 4, 1, 9);

    testsupport_parser_node_create(fc, fncall, 1, 4, 1, 11, fn_name, NULL, NULL);
    ast_node_add_child(bnode1, fc);

    testsupport_parser_node_create(cond1, condbranch, 0, 3, 2, 0,
                                   cmp_exp, bnode1);



    struct ast_node *id2 = testsupport_parser_identifier_create(2, 8, 2, 8);
    testsupport_parser_constant_create(cnum2, 2, 13, 2, 14, integer, 50);
    testsupport_parser_node_create(op1, binaryop, 2, 8, 2, 14,
                                   BINARYOP_CMP_EQ, id2, cnum2);
    struct ast_node *fn_name2 = testsupport_parser_identifier_create(
        2, 19, 2, 25);
    testsupport_parser_node_create(fc2, fncall, 2, 19, 2, 27,
                                   fn_name2, NULL, NULL);
    testsupport_parser_node_create(cmp_exp2, binaryop, 2, 8, 2, 27,
                                   BINARYOP_LOGIC_AND, op1, fc2);

    testsupport_parser_block_create(bnode2, 2, 30, 4, 0);
    testsupport_parser_string_literal_create(sliteral1, 3, 4, 3, 8);
    ast_node_add_child(bnode2, sliteral1);
    testsupport_parser_node_create(cond2, condbranch, 2, 8, 4, 0,
                                   cmp_exp2, bnode2);


    testsupport_parser_constant_create(cnum3, 4, 8, 4, 8, integer, 5);
    struct ast_node *id3 = testsupport_parser_identifier_create(4, 12, 4, 20);
    testsupport_parser_node_create(cmp_exp3, binaryop, 4, 8, 4, 20,
                                   BINARYOP_CMP_LT, cnum3, id3);
    testsupport_parser_block_create(bnode3, 4, 23, 6, 0);
    testsupport_parser_constant_create(cnum4, 5, 4, 5, 9, float, 283.23);
    ast_node_add_child(bnode3, cnum4);
    testsupport_parser_node_create(cond3, condbranch, 4, 8, 6, 0,
                                   cmp_exp3, bnode3);


    testsupport_parser_block_create(bnode4, 6, 7, 8, 0);
    testsupport_parser_constant_create(cnum5, 7, 4, 7, 5, integer, 55);
    testsupport_parser_constant_create(cnum6, 7, 9, 7, 12, float, 2.31);
    testsupport_parser_node_create(op3, binaryop, 7, 4, 7, 12,
                                   BINARYOP_ADD, cnum5, cnum6);
    ast_node_add_child(bnode4, op3);

    // the second elif
    testsupport_parser_node_create(if3, ifexpr, 4, 2, 8, 0, cond3, bnode4);
    // the first elif
    testsupport_parser_node_create(if2, ifexpr, 2, 2, 8, 0, cond2, if3);
    // the first if
    testsupport_parser_node_create(ifx, ifexpr, 0, 0, 8, 0, cond1, if2);


    ck_test_parse_as(n, ifexpr, "if_expression", ifx, TOKEN_KW_IF);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST

START_TEST(test_acc_ifexpr_ambiguous_less_than_or_generic) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a < 42 {\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 3, 0, 3);
    testsupport_parser_constant_create(cnum, 0, 7, 0, 8, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, 0, 3, 0, 8,
                                   BINARYOP_CMP_LT, id1, cnum);


    testsupport_parser_block_create(bnode, 0, 10, 1, 0);
    testsupport_parser_node_create(cond, condbranch, 0, 3, 1, 0,
                                   cmp_exp, bnode);

    testsupport_parser_node_create(ifx, ifexpr, 0, 0, 1, 0, cond, NULL);

    ck_test_parse_as(n, ifexpr, "if_expression", ifx, TOKEN_KW_IF);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST

START_TEST(test_acc_ifexpr_errors_1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if "
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(ifexpr, TOKEN_KW_IF);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an expression after 'if'",
            0, 1),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_ifexpr_errors_2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a > 25 \n"
        " do_sth()\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(ifexpr, TOKEN_KW_IF);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a block after \"if\"'s conditional expression",
            0, 8),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_ifexpr_errors_3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a > 25 {\n"
        " do_sth()\n"
        "} else {"
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(ifexpr, TOKEN_KW_IF);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an expression or a '}' at block end",
            2, 7),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a block after 'else'",
            2, 5),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_ifexpr_errors_4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a > 25 {\n"
        " do_sth()\n"
        "} elif "
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(ifexpr, TOKEN_KW_IF);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an expression after 'elif'",
            2, 5),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_ifexpr_errors_5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a > 25 {\n"
        " do_sth()\n"
        "} elif 55 == foo_value \n"
        "  10 + 231\n"
        "} else {\n"
        "  else_action()\n"
        "}"
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(ifexpr, TOKEN_KW_IF);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a block after \"elif\"'s conditional expression",
            2, 21),
    };
    ck_assert_parser_errors(errors);
}END_TEST

Suite *parser_ifexpr_suite_create(void)
{
    Suite *s = suite_create("parser_if_expression");

    TCase *ifp = tcase_create("if_expression_parsing");
    tcase_add_checked_fixture(ifp, setup_front_tests, teardown_front_tests);
    tcase_add_test(ifp, test_acc_ifexpr_1);
    tcase_add_test(ifp, test_acc_ifexpr_2);
    tcase_add_test(ifp, test_acc_ifexpr_3);
    tcase_add_test(ifp, test_acc_ifexpr_4);
    tcase_add_test(ifp, test_acc_ifexpr_ambiguous_less_than_or_generic);

    TCase *iferr = tcase_create("if_expression_parsing_errors");
    tcase_add_checked_fixture(iferr, setup_front_tests, teardown_front_tests);
    tcase_add_test(iferr, test_acc_ifexpr_errors_1);
    tcase_add_test(iferr, test_acc_ifexpr_errors_2);
    tcase_add_test(iferr, test_acc_ifexpr_errors_3);
    tcase_add_test(iferr, test_acc_ifexpr_errors_4);
    tcase_add_test(iferr, test_acc_ifexpr_errors_5);

    suite_add_tcase(s, ifp);
    suite_add_tcase(s, iferr);

    return s;
}
