#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>
#include <ast/function.h>
#include <ast/operators.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_typecheck_variable_declarations) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:u64 = 523432\n"
        "b:u64 = 123 + b\n"
        "s:string = \"Foo\" + \"Bar\"\n"
        "d:f32 = 98 / 3.21\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_negative_int_variable_declarations) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32 = -23432\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_complex_type_in_variable_declaration) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:(b:int | c:string)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_foreign_import) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "foreign_import rf_stdlib_print_int64(i64), rf_stdlib_print_string(string)\n"
        "{\n"
        "rf_stdlib_print_int64(3)\n"
        "rf_stdlib_print_string(\"hello\")\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_valid_custom_type_and_fncall1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something(a:person) -> string\n"
        "{\n"
        "return \"something\""
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_valid_custom_type_and_fncall2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something(a:person, b:u64) -> string\n"
        "{\n"
        "return \"something\" + a.name"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_valid_custom_type_constructor) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }"
        "fn do_something() -> string\n"
        "{\n"
        "a:person = person(\"Celina\", 18)\n"
        "return a.name"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST(test_typecheck_valid_custom_sum_type_constructor) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string | id:u32 }"
        "fn do_something()\n"
        "{\n"
        "a:person = person(\"Celina\")\n"
        "b:person = person(13)\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    struct ast_node *fn_impl = ast_node_get_child(front_testdriver_root(), 1);
    ck_assert(fn_impl->type == AST_FUNCTION_IMPLEMENTATION);
    struct ast_node *block = ast_fnimpl_body_get(fn_impl);
    struct ast_node *bop1 = ast_node_get_child(block, 0);
    struct ast_node *bop2 = ast_node_get_child(block, 1);
    ck_assert(ast_node_is_specific_binaryop(bop1, BINARYOP_ASSIGN));
    ck_assert(ast_node_is_specific_binaryop(bop2, BINARYOP_ASSIGN));
    struct ast_node *ctor1 = ast_binaryop_right(bop1);
    struct ast_node *ctor2 = ast_binaryop_right(bop2);
    ck_assert(ctor1->type = AST_FUNCTION_CALL);
    ck_assert(ctor2->type = AST_FUNCTION_CALL);
    struct type *t_string = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_STRING);
    struct type *t_u32 = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_UINT_32);
    ck_assert(type_compare(ctor1->fncall.params_type, t_string, TYPECMP_IDENTICAL));
    ck_assert(type_compare(ctor2->fncall.params_type, t_u32, TYPECMP_IDENTICAL));
} END_TEST

START_TEST(test_typecheck_valid_custom_sum_type_constructor2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, b:u64 | id:u32, foo:f64}"
        "fn do_something()\n"
        "{\n"
        "a:person = person(\"Celina\", 53342)\n"
        "b:person = person(13, 54.231)\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();

    struct ast_node *fn_impl = ast_node_get_child(front_testdriver_root(), 1);
    ck_assert(fn_impl->type == AST_FUNCTION_IMPLEMENTATION);
    struct ast_node *block = ast_fnimpl_body_get(fn_impl);
    struct ast_node *bop1 = ast_node_get_child(block, 0);
    struct ast_node *bop2 = ast_node_get_child(block, 1);
    ck_assert(ast_node_is_specific_binaryop(bop1, BINARYOP_ASSIGN));
    ck_assert(ast_node_is_specific_binaryop(bop2, BINARYOP_ASSIGN));
    struct ast_node *ctor1 = ast_binaryop_right(bop1);
    struct ast_node *ctor2 = ast_binaryop_right(bop2);
    ck_assert(ctor1->type = AST_FUNCTION_CALL);
    ck_assert(ctor2->type = AST_FUNCTION_CALL);

    struct type *t_string = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_STRING);
    struct type *t_u64 = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_UINT_64);

    struct type *t_u32 = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_UINT_32);
    struct type *t_f64 = testsupport_analyzer_type_create_simple_elementary(ELEMENTARY_TYPE_FLOAT_64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_string,
                                                                      t_u64);
    struct type *t_prod_2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_u32,
                                                                     t_f64);
    ck_assert(type_compare(ctor1->fncall.params_type, t_prod_1, TYPECMP_IDENTICAL));
    ck_assert(type_compare(ctor2->fncall.params_type, t_prod_2, TYPECMP_IDENTICAL));
} END_TEST

START_TEST(test_typecheck_invalid_custom_type_constructor) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:u32 }\n"
        "fn do_something() -> string\n"
        "{\n"
        "a:person = person(\"Celina\")\n"
        "return a.name"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "constructor person() is called with argument type of "
            "\"string\" which does not match the expected type of \"string,u32\".",
            3, 11, 3, 26)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typecheck_invalid_type_in_typedecl) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person { name:string, age:if32 }\n"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Type \"if32\" is not defined",
            0, 31, 0, 34)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST (test_typecheck_valid_assignment_from_block1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        \"a_string_literal\"\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_valid_assignment_from_block2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:u64, b:string }\n"
        "{\n"
        "    a:foo = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        c:foo = foo(565, \"Berlin\")\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST

START_TEST (test_typecheck_invalid_assignment_from_block1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        \"a_string_literal\"\n"
        "        b:u32 = 13 + 25\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"u32\" to \"string\". Unable to convert from \"u32\" to \"string\".",
            1, 4, 5, 4),
    };
    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST (test_typecheck_invalid_assignment_from_block2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:u64, b:string }\n"
        "{\n"
        "    a:string = \n"
        "    {\n"
        "        b:u32 = 13 + 25\n"
        "        c:foo = foo(565, \"Berlin\")\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Assignment between incompatible types. Can't assign "
            "\"foo\" to \"string\".",
            2, 4, 6, 4),
    };
    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST (test_typecheck_valid_if_stmt) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "    a:u64 = 1453\n"
        "    b:u64"
        "    if a == 1453 { b = 1 } else { b = 0 }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_typecheck_ok();
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *t_typecheck_misc = tcase_create("typecheck_misc");
    tcase_add_checked_fixture(t_typecheck_misc,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_typecheck_misc, test_typecheck_variable_declarations);
    tcase_add_test(t_typecheck_misc, test_typecheck_negative_int_variable_declarations);
    tcase_add_test(t_typecheck_misc, test_typecheck_complex_type_in_variable_declaration);
    tcase_add_test(t_typecheck_misc, test_typecheck_foreign_import);
    // TODO: Test where there are errors in two different parts of the code
    //       to assert the continuation of the traversal works

    TCase *t_custom_types_val = tcase_create("typecheck_valid_custom_types");
    tcase_add_checked_fixture(t_custom_types_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall1);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_and_fncall2);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_type_constructor);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_sum_type_constructor);
    tcase_add_test(t_custom_types_val, test_typecheck_valid_custom_sum_type_constructor2);

    TCase *t_custom_types_inv = tcase_create("typecheck_invalid_custom_types");
    tcase_add_checked_fixture(t_custom_types_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_custom_types_inv, test_typecheck_invalid_custom_type_constructor);
    tcase_add_test(t_custom_types_inv, test_typecheck_invalid_type_in_typedecl);

    TCase *t_block_val = tcase_create("typecheck_valid_blocks");
    tcase_add_checked_fixture(t_block_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_block_val, test_typecheck_valid_assignment_from_block1);
    tcase_add_test(t_block_val, test_typecheck_valid_assignment_from_block2);


    TCase *t_block_inv = tcase_create("typecheck_invalid_blocks");
    tcase_add_checked_fixture(t_block_inv,
                              setup_analyzer_tests_with_filelog,
                              teardown_analyzer_tests);
    tcase_add_test(t_block_inv, test_typecheck_invalid_assignment_from_block1);
    tcase_add_test(t_block_inv, test_typecheck_invalid_assignment_from_block2);

    TCase *t_if_val = tcase_create("typecheck_valid_if");
    tcase_add_checked_fixture(t_if_val,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_if_val, test_typecheck_valid_if_stmt);

    suite_add_tcase(s, t_typecheck_misc);
    suite_add_tcase(s, t_custom_types_val);
    suite_add_tcase(s, t_custom_types_inv);
    suite_add_tcase(s, t_block_val);
    suite_add_tcase(s, t_block_inv);
    suite_add_tcase(s, t_if_val);

    return s;
}


