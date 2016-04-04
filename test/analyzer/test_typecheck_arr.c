#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>

#include <ast/function.h>
#include <ast/matchexpr.h>
#include <ast/operators.h>
#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_array_type1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:i8[]\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    int64_t dims[] = {-1};
    testsupport_analyzer_type_create_elementary(
        t_i8,
        ELEMENTARY_TYPE_INT_8,
        dims
    );
    struct ast_node *typedesc = ast_node_get_child(front_testdriver_module()->node, 0);
    const struct type *n_type = ast_node_get_type(typedesc);
    ck_assert_msg(n_type, "Type should not be NULL");
    ck_assert_msg(
        type_compare(n_type, t_i8, TYPECMP_IDENTICAL),
        "Node type comparison failure"
    );
} END_TEST

START_TEST(test_array_type2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:u64[42]\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    int64_t dims[] = {42};
    testsupport_analyzer_type_create_elementary(
        t_u64,
        ELEMENTARY_TYPE_UINT_64,
        dims
    );
    struct ast_node *typedesc = ast_node_get_child(front_testdriver_module()->node, 0);
    const struct type *n_type = ast_node_get_type(typedesc);
    ck_assert_msg(n_type, "Type should not be NULL");
    ck_assert_msg(
        type_compare(n_type, t_u64, TYPECMP_IDENTICAL),
        "Node type comparison failure"
    );
} END_TEST

START_TEST(test_array_type3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:u64[42][13][24]\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    int64_t dims[] = {42, 13, 24};
    testsupport_analyzer_type_create_elementary(
        t_u64,
        ELEMENTARY_TYPE_UINT_64,
        dims
    );
    struct ast_node *typedesc = ast_node_get_child(front_testdriver_module()->node, 0);
    const struct type *n_type = ast_node_get_type(typedesc);
    ck_assert_msg(n_type, "Type should not be NULL");
    ck_assert_msg(
        type_compare(n_type, t_u64, TYPECMP_IDENTICAL),
        "Node type comparison failure"
    );
} END_TEST

START_TEST(test_array_type4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u64[] = [1, 2, 3]\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    int64_t dims[] = {3};
    testsupport_analyzer_type_create_elementary(
        t_u64,
        ELEMENTARY_TYPE_UINT_64,
        dims
    );
    struct ast_node *block = ast_node_get_child(front_testdriver_module()->node, 0);
    struct ast_node *blist = ast_binaryop_right(ast_node_get_child(block, 0));
    const struct type *blist_type = ast_node_get_type(blist);
    ck_assert_msg(blist_type, "bracket type should not be NULL");
    ck_assert_msg(
        type_compare(blist_type, t_u64, TYPECMP_GENERIC),
        "Bracket type comparison failure"
    );

    struct ast_node *vardecl = ast_binaryop_left(ast_node_get_child(block, 0));
    const struct type *vardecl_type = ast_node_get_type(vardecl);
    ck_assert_msg(vardecl_type, "bracket type should not be NULL");
    ck_assert_msg(
        type_compare(vardecl_type, t_u64, TYPECMP_IDENTICAL),
        "Bracket type comparison failure"
    );
} END_TEST

START_TEST(test_array_type_compare1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u64[3] = [1, 2, 3]\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    int64_t dims[] = {3};
    testsupport_analyzer_type_create_elementary(
        t_u64,
        ELEMENTARY_TYPE_UINT_64,
        dims
    );
    struct ast_node *block = ast_node_get_child(front_testdriver_module()->node, 0);
    struct ast_node *blist = ast_binaryop_right(ast_node_get_child(block, 0));
    const struct type *blist_type = ast_node_get_type(blist);
    ck_assert_msg(blist_type, "bracket type should not be NULL");
    ck_assert_msg(
        type_compare(blist_type, t_u64, TYPECMP_GENERIC),
        "Bracket type comparison failure"
    );

    struct ast_node *vardecl = ast_binaryop_left(ast_node_get_child(block, 0));
    const struct type *vardecl_type = ast_node_get_type(vardecl);
    ck_assert_msg(vardecl_type, "bracket type should not be NULL");
    ck_assert_msg(
        type_compare(vardecl_type, t_u64, TYPECMP_IDENTICAL),
        "Bracket type comparison failure"
    );
} END_TEST

Suite *analyzer_typecheck_array_suite_create(void)
{
    Suite *s = suite_create("typecheck_arrays");

    TCase *tc1 = tcase_create("typecheck_array_types");
    tcase_add_checked_fixture(
        tc1,
        setup_analyzer_tests_no_stdlib,
        teardown_analyzer_tests
    );
    tcase_add_test(tc1, test_array_type1);
    tcase_add_test(tc1, test_array_type2);
    tcase_add_test(tc1, test_array_type3);
    tcase_add_test(tc1, test_array_type4);

    TCase *tc2 = tcase_create("typecheck_array_type_comparison");
    tcase_add_checked_fixture(
        tc2,
        setup_analyzer_tests,
        teardown_analyzer_tests
    );
    tcase_add_test(tc2, test_array_type_compare1);

    suite_add_tcase(s, tc1);
    suite_add_tcase(s, tc2);

    return s;
}


