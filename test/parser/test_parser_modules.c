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
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics {\n"
        "fn foo() -> i32 { return -1 }\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_graphics = testsupport_parser_identifier_create(file,
                                                                        0, 7, 0, 14);
    
    struct ast_node *id_foo = testsupport_parser_identifier_create(file,
                                                                   1, 3, 1, 5);
    testsupport_parser_xidentifier_create_simple(id_i32, file, 1, 12, 1, 14);
    testsupport_parser_node_create(decl, fndecl, file, 1, 0, 1, 14,
                                   FNDECL_PARTOF_IMPL, id_foo, NULL, NULL, id_i32);
    testsupport_parser_block_create(function_body, file, 1, 16, 1, 28);
    testsupport_parser_constant_create(c_m1, file,
                                       1, 25, 1, 26, integer, -1);
    testsupport_parser_node_create(ret_stmt, returnstmt, file, 1, 18, 1, 26, c_m1);
    ast_node_add_child(function_body, ret_stmt);
    testsupport_parser_node_create(function, fnimpl, file,
                                   1, 0, 1, 28, decl, function_body);
    testsupport_parser_node_create(mod, module, file,
                                   0, 0, 2, 0, id_graphics, NULL);    
    ast_node_add_child(mod, function);

    ck_test_parse_as(n, module, d, "module", mod);

    ast_node_destroy(n);
    ast_node_destroy(mod);
} END_TEST

START_TEST (test_acc_module_with_args) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module graphics(g:geometry, buffer_size:u64) {\n"
        "import g\n"
        "fn foo() -> i32 { return -1 }\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = d->front.file;

    struct ast_node *id_graphics = testsupport_parser_identifier_create(file,
                                                                        0, 7, 0, 14);
    
    struct ast_node *id_g = testsupport_parser_identifier_create(file,
                                                                 0, 16, 0, 16);
    testsupport_parser_xidentifier_create_simple(id_geometry, file, 0, 18, 0, 25);
    testsupport_parser_node_create(t1, typeleaf, file, 0, 16, 0, 25, id_g, id_geometry);
    struct ast_node *id_buffer = testsupport_parser_identifier_create(file,
                                                                      0, 28, 0, 38);
    testsupport_parser_xidentifier_create_simple(id_u64, file, 0, 40, 0, 42);
    testsupport_parser_node_create(t2, typeleaf, file, 0, 28, 0, 42, id_buffer, id_u64);
    testsupport_parser_node_create(modargs, typeop, file, 0, 16, 0, 42,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id_g1 = testsupport_parser_identifier_create(file,
                                                                  1, 7, 1, 7);
    testsupport_parser_node_create(import_stmt, import, file, 1, 0, 1, 7, false);
    ast_node_add_child(import_stmt, id_g1);
    
    
    struct ast_node *id_foo = testsupport_parser_identifier_create(file,
                                                                   2, 3, 2, 5);
    testsupport_parser_xidentifier_create_simple(id_i32, file, 2, 12, 2, 14);
    testsupport_parser_node_create(decl, fndecl, file, 2, 0, 2, 14,
                                   FNDECL_PARTOF_IMPL, id_foo, NULL, NULL, id_i32);
    testsupport_parser_block_create(function_body, file, 2, 16, 2, 28);
    testsupport_parser_constant_create(c_m1, file,
                                       2, 25, 2, 26, integer, -1);
    testsupport_parser_node_create(ret_stmt, returnstmt, file, 2, 18, 2, 26, c_m1);
    ast_node_add_child(function_body, ret_stmt);
    testsupport_parser_node_create(function, fnimpl, file,
                                   2, 0, 2, 28, decl, function_body);

    testsupport_parser_node_create(mod, module, file,
                                   0, 0, 3, 0, id_graphics, modargs);    
    ast_node_add_child(mod, import_stmt);
    ast_node_add_child(mod, function);
    ck_test_parse_as(n, module, d, "module", mod);

    ast_node_destroy(n);
    ast_node_destroy(mod);
} END_TEST


Suite *parser_modules_suite_create(void)
{
    Suite *s = suite_create("parser_modules");

    TCase *tc1 = tcase_create("parser_module_decls");
    tcase_add_checked_fixture(tc1, setup_front_tests, teardown_front_tests);
    tcase_add_test(tc1, test_acc_module_simple);
    tcase_add_test(tc1, test_acc_module_with_args);

    suite_add_tcase(s, tc1);
    return s;
}
