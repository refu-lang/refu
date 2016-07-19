#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rfbase/string/core.h>

#include <parser/parser.h>
#include "../../src/parser/recursive_descent/forexpr.h"
#include <ast/forexpr.h>
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

START_TEST(test_acc_forexpr_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "for a in array {\n"
        "    do_sth(a)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 4, 0, 4);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 9, 0, 13);
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
            "Expected an identifier for the iterable after 'in'",
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

Suite *parser_forexpr_suite_create(void)
{
    Suite *s = suite_create("parser_for_expression");

    TCase *fep = tcase_create("for_expression_parsing");
    tcase_add_checked_fixture(fep, setup_front_tests, teardown_front_tests);
    tcase_add_test(fep, test_acc_forexpr_1);

    TCase *ferr = tcase_create("for_expression_parsing_errors");
    tcase_add_checked_fixture(ferr, setup_front_tests, teardown_front_tests);
    tcase_add_test(ferr, test_acc_forexpr_errors_1);
    tcase_add_test(ferr, test_acc_forexpr_errors_2);
    tcase_add_test(ferr, test_acc_forexpr_errors_3);
    tcase_add_test(ferr, test_acc_forexpr_errors_4);

    suite_add_tcase(s, fep);
    suite_add_tcase(s, ferr);

    return s;
}
