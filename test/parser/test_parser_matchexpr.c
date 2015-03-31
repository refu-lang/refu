#include <check.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/matchexpr.h"
#include <ast/matchexpr.h>
#include <ast/string_literal.h>
#include <ast/constants.h>
#include <ast/type.h>
#include <lexer/lexer.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_matchexpr_1case) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "match a {\n"
        "    _ => \"only_case\"\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 6, 0, 6);
    testsupport_parser_node_create(mexpr, matchexpr, file, 0, 0, 2, 0, id_a);

    testsupport_parser_xidentifier_create_simple(id_wildcard, file, 1, 4, 1, 4);
    testsupport_parser_string_literal_create(sliteral1, file,
                                             1, 9, 1, 19);
    testsupport_parser_node_create(mcase, matchcase, file, 1, 4, 1, 19,
                                   id_wildcard, sliteral1);
    ast_node_add_child(mexpr, mcase);

    ck_test_parse_as(n, matchexpr, d, "match expression", mexpr, true);

    ast_node_destroy(n);
    ast_node_destroy(mexpr);
}END_TEST

START_TEST(test_acc_matchexpr_2cases) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "match a {\n"
        "    a:i32 => a\n"
        "    _     => \"others_cases\"\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 6, 0, 6);
    testsupport_parser_node_create(mexpr, matchexpr, file, 0, 0, 3, 0, id_a);

    struct ast_node *id_ma = testsupport_parser_identifier_create(file, 1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_i32, file, 1, 6, 1, 8);
    testsupport_parser_node_create(ai32, typedesc, file, 1, 4, 1, 8, id_ma, id_i32);
    struct ast_node *id_c1a = testsupport_parser_identifier_create(file,
                                                                   1, 13, 1, 13);
    testsupport_parser_node_create(mcase1, matchcase, file, 1, 4, 1, 13,
                                   ai32, id_c1a);
    ast_node_add_child(mexpr, mcase1);

    testsupport_parser_xidentifier_create_simple(id_wildcard, file, 2, 4, 2, 4);
    testsupport_parser_string_literal_create(sliteral1, file,
                                             2, 13, 2, 26);
    testsupport_parser_node_create(mcase2, matchcase, file, 2, 4, 2, 26,
                                   id_wildcard, sliteral1);
    ast_node_add_child(mexpr, mcase2);
    

    ck_test_parse_as(n, matchexpr, d, "match expression", mexpr, true);

    ast_node_destroy(n);
    ast_node_destroy(mexpr);
}END_TEST


START_TEST(test_acc_matchexpr_3cases) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "match a {\n"
        "    a:i32 => a\n"
        "    f32   => \"float\"\n"
        "    _     => \"others_cases\"\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_a = testsupport_parser_identifier_create(file,
                                                                 0, 6, 0, 6);
    testsupport_parser_node_create(mexpr, matchexpr, file, 0, 0, 4, 0, id_a);

    struct ast_node *id_ma = testsupport_parser_identifier_create(file, 1, 4, 1, 4);
    testsupport_parser_xidentifier_create_simple(id_i32, file, 1, 6, 1, 8);
    testsupport_parser_node_create(ai32, typedesc, file, 1, 4, 1, 8, id_ma, id_i32);
    struct ast_node *id_c1a = testsupport_parser_identifier_create(file,
                                                                   1, 13, 1, 13);
    testsupport_parser_node_create(mcase1, matchcase, file, 1, 4, 1, 13,
                                   ai32, id_c1a);
    ast_node_add_child(mexpr, mcase1);

    testsupport_parser_xidentifier_create_simple(id_f32, file, 2, 4, 2, 6);
    testsupport_parser_string_literal_create(sliteral1, file,
                                             2, 13, 2, 19);
    testsupport_parser_node_create(mcase2, matchcase, file, 2, 4, 2, 19,
                                   id_f32, sliteral1);
    ast_node_add_child(mexpr, mcase2);
    
    testsupport_parser_xidentifier_create_simple(id_wildcard, file, 3, 4, 3, 4);
    testsupport_parser_string_literal_create(sliteral2, file,
                                             3, 13, 3, 26);
    testsupport_parser_node_create(mcase3, matchcase, file, 3, 4, 3, 26,
                                   id_wildcard, sliteral2);
    ast_node_add_child(mexpr, mcase3);
    

    ck_test_parse_as(n, matchexpr, d, "match expression", mexpr, true);

    ast_node_destroy(n);
    ast_node_destroy(mexpr);
}END_TEST


Suite *parser_matchexpr_suite_create(void)
{
    Suite *s = suite_create("parser_matchexpr");

    TCase *tc1 = tcase_create("parser_matchexpr_simple");
    tcase_add_checked_fixture(tc1, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc1, test_acc_matchexpr_1case);
    tcase_add_test(tc1, test_acc_matchexpr_2cases);
    tcase_add_test(tc1, test_acc_matchexpr_3cases);

    suite_add_tcase(s, tc1);

    return s;
}
