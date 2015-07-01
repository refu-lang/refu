/**
 * Tests for parsing of ast node not having their own test file
 */

#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/module.h"
#include <ast/type.h>
#include <ast/module.h>
#include <ast/block.h>
#include <ast/function.h>
#include <ast/returnstmt.h>
#include <ast/constants.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST (test_acc_module_simple) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics {\n"
        "fn foo() -> i32 { return -1 }\n"
        "}");
    front_testdriver_new_main_source(&s);

    struct ast_node *id_graphics = testsupport_parser_identifier_create(0, 7, 0, 14);
    
    struct ast_node *id_foo = testsupport_parser_identifier_create(1, 3, 1, 5);
    testsupport_parser_xidentifier_create_simple(id_i32, 1, 12, 1, 14);
    testsupport_parser_node_create(decl, fndecl, 1, 0, 1, 14,
                                   FNDECL_PARTOF_IMPL, id_foo, NULL, NULL, id_i32);
    testsupport_parser_block_create(function_body, 1, 16, 1, 28);
    testsupport_parser_constant_create(c_m1, 1, 25, 1, 26, integer, -1);
    testsupport_parser_node_create(ret_stmt, returnstmt, 1, 18, 1, 26, c_m1);
    ast_node_add_child(function_body, ret_stmt);
    testsupport_parser_node_create(function, fnimpl, 1, 0, 1, 28, decl, function_body);
    testsupport_parser_node_create(mod, module, 0, 0, 2, 0, id_graphics, NULL);    
    ast_node_add_child(mod, function);

    ck_test_parse_as(n, module, "module", mod);
    
    ast_node_destroy(n);
    ast_node_destroy(mod);
} END_TEST

START_TEST (test_acc_module_with_args) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics(g:geometry, buffer_size:u64) {\n"
        "import g\n"
        "fn foo() -> i32 { return -1 }\n"
        "}");
    front_testdriver_new_main_source(&s);

    struct ast_node *id_graphics = testsupport_parser_identifier_create(0, 7, 0, 14);
    
    struct ast_node *id_g = testsupport_parser_identifier_create(0, 16, 0, 16);
    testsupport_parser_xidentifier_create_simple(id_geometry, 0, 18, 0, 25);
    testsupport_parser_node_create(t1, typeleaf, 0, 16, 0, 25, id_g, id_geometry);
    struct ast_node *id_buffer = testsupport_parser_identifier_create(0, 28, 0, 38);
    testsupport_parser_xidentifier_create_simple(id_u64, 0, 40, 0, 42);
    testsupport_parser_node_create(t2, typeleaf, 0, 28, 0, 42, id_buffer, id_u64);
    testsupport_parser_node_create(modargs, typeop, 0, 16, 0, 42,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id_g1 = testsupport_parser_identifier_create(1, 7, 1, 7);
    testsupport_parser_node_create(import_stmt, import, 1, 0, 1, 7, false);
    ast_node_add_child(import_stmt, id_g1);
    
    
    struct ast_node *id_foo = testsupport_parser_identifier_create(2, 3, 2, 5);
    testsupport_parser_xidentifier_create_simple(id_i32, 2, 12, 2, 14);
    testsupport_parser_node_create(decl, fndecl, 2, 0, 2, 14,
                                   FNDECL_PARTOF_IMPL, id_foo, NULL, NULL, id_i32);
    testsupport_parser_block_create(function_body, 2, 16, 2, 28);
    testsupport_parser_constant_create(c_m1, 2, 25, 2, 26, integer, -1);
    testsupport_parser_node_create(ret_stmt, returnstmt, 2, 18, 2, 26, c_m1);
    ast_node_add_child(function_body, ret_stmt);
    testsupport_parser_node_create(function, fnimpl, 2, 0, 2, 28, decl, function_body);

    testsupport_parser_node_create(mod, module, 0, 0, 3, 0, id_graphics, modargs);    
    ast_node_add_child(mod, import_stmt);
    ast_node_add_child(mod, function);
    ck_test_parse_as(n, module, "module", mod);

    ast_node_destroy(n);
    ast_node_destroy(mod);
} END_TEST

START_TEST (test_acc_module_fail1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("module () {}");
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the module name after 'module'",
            0, 7)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("module graphics");
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a '(' or a '{' at module declaration",
            0, 14)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics {\n"
        "a:i32 = 5\n"
        "}\n"
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a module statement or '}'",
            1, 0)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics {\n"
        "fn foo() -> i32{ return 42 }\n"
        "\n"
    );
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a module statement or '}'",
            1, 27)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("module graphics (a:) {}");
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "expected 'const' or identifier or '(' after ':'",
            0, 19),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected either a type description for the module's arguments or ')' after '('",
            0, 16),
        
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail6) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("module graphics (a:i32 {}");
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected ')' at module declaration after type description",
            0, 23),
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST (test_acc_module_fail7) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("module graphics ( {}");
    front_testdriver_new_main_source(&s);

    ck_test_fail_parse_as(module);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected ')' at module declaration after '('",
            0, 18),
    };
    ck_assert_parser_errors(errors);
} END_TEST

Suite *parser_modules_suite_create(void)
{
    Suite *s = suite_create("parser_modules");

    TCase *tc1 = tcase_create("parser_module_decls");
    tcase_add_checked_fixture(tc1, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc1, test_acc_module_simple);
    tcase_add_test(tc1, test_acc_module_with_args);

    TCase *tc2 = tcase_create("parser_module_decls_failures");
    tcase_add_checked_fixture(tc2, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc2, test_acc_module_fail1);
    tcase_add_test(tc2, test_acc_module_fail2);
    tcase_add_test(tc2, test_acc_module_fail3);
    tcase_add_test(tc2, test_acc_module_fail4);
    tcase_add_test(tc2, test_acc_module_fail5);
    tcase_add_test(tc2, test_acc_module_fail6);
    tcase_add_test(tc2, test_acc_module_fail7);

    suite_add_tcase(s, tc1);
    suite_add_tcase(s, tc2);
    return s;
}
