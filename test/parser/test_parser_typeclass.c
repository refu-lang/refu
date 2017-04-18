#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rfbase/string/core.h>

#include <parser/parser.h>
#include "../../src/parser/recursive_descent/typeclass.h"
#include <ast/function.h>
#include <ast/type.h>
#include <ast/typeclass.h>
#include <ast/generics.h>
#include <ast/block.h>
#include <ast/operators.h>
#include <ast/constants.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typeclass_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand {\n"
        "fn dosth(a:i32) -> i32\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 6, 0, 17);
    testsupport_parser_node_create(tclass, typeclass, 0, 0, 2, 0,
                                   name,
                                   NULL);
    struct ast_node *fn_name = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id2, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typeleaf, 1, 9, 1, 13, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 1, 19, 1, 21);
    testsupport_parser_node_create(fn, fndecl, 1, 0, 1, 21,
                                   FNDECL_PARTOF_TYPECLASS,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id3
    );
    ast_node_add_child(tclass, fn);

    ck_test_parse_as(n, typeclass, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_with_generics) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand <Type foo> {\n"
        "fn dosth(a:foo) -> i32\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 6, 0, 17);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 20, 0, 23);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 25, 0, 27);
    struct ast_node *gtype = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, 0, 19, 0, 28);
    ast_node_add_child(genr, gtype);

    testsupport_parser_node_create(tclass, typeclass, 0, 0, 2, 0,
                                   name,
                                   genr);

    struct ast_node *fn_name = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *id3 = testsupport_parser_identifier_create(1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id4, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typeleaf, 1, 9, 1, 13, id3, id4);
    testsupport_parser_xidentifier_create_simple(id5, 1, 19, 1, 21);
    testsupport_parser_node_create(fn, fndecl, 1, 0, 1, 21,
                                   FNDECL_PARTOF_TYPECLASS,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id5
    );
    ast_node_add_child(tclass, fn);

    ck_test_parse_as(n, typeclass, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand <Type foo> {\n"
        "fn dosth(a:foo) -> i32\n"
        "fn dosth_else() -> i32\n"
        "fn act(a:u64) -> foo\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 6, 0, 17);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 20, 0, 23);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 25, 0, 27);
    struct ast_node *gtype = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, 0, 19, 0, 28);
    ast_node_add_child(genr, gtype);

    testsupport_parser_node_create(tclass, typeclass, 0, 0, 4, 0,
                                   name,
                                   genr);

    struct ast_node *fn_name1 = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *id3 = testsupport_parser_identifier_create(1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id4, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typeleaf, 1, 9, 1, 13, id3, id4);
    testsupport_parser_xidentifier_create_simple(id5, 1, 19, 1, 21);
    testsupport_parser_node_create(fn1, fndecl, 1, 0, 1, 21,
                                   FNDECL_PARTOF_TYPECLASS,
                                   fn_name1,
                                   NULL,
                                   t1,
                                   id5
    );
    ast_node_add_child(tclass, fn1);

    struct ast_node *fn_name2 = testsupport_parser_identifier_create(2, 3, 2, 12);
    testsupport_parser_xidentifier_create_simple(id6, 2, 19, 2, 21);
    testsupport_parser_node_create(fn2, fndecl, 2, 0, 2, 21,
                                   FNDECL_PARTOF_TYPECLASS,
                                   fn_name2,
                                   NULL,
                                   NULL,
                                   id6
    );
    ast_node_add_child(tclass, fn2);

    struct ast_node *fn_name3 = testsupport_parser_identifier_create(3, 3, 3, 5);
    struct ast_node *id7 = testsupport_parser_identifier_create(3, 7, 3, 7);
    testsupport_parser_xidentifier_create_simple(id8, 3, 9, 3, 11);
    testsupport_parser_node_create(t2, typeleaf, 3, 7, 3, 11, id7, id8);
    testsupport_parser_xidentifier_create_simple(id9, 3, 17, 3, 19);
    testsupport_parser_node_create(fn3, fndecl, 3, 0, 3, 19,
                                   FNDECL_PARTOF_TYPECLASS,
                                   fn_name3,
                                   NULL,
                                   t2,
                                   id9
    );
    ast_node_add_child(tclass, fn3);

    ck_test_parse_as(n, typeclass, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_err1) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the typeclass name after 'class'",
            0, 6)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeclass_err2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected at least one function declaration inside the body of "
            "typeclass \"pointers\" after '{'",
            0, 15)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeclass_err3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class adder <Type a {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected either a ',' or a '>' at generic declaration",
            0, 20),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a generic declaration for typeclass \"adder\" "
            "after identifier",
            0, 10)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeclass_err4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers \n"
        "fn do_sth()\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected '{' at \"pointers\" typeclass "
            "declaration after identifier",
            0, 13)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeclass_err5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn do_sth()");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected '}' at the end of \"pointers\" typeclass declaration",
            1, 10)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeclass_err6) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn dosth(\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeclass);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected ')' at function declaration after '('",
            1, 8),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a proper function declaration "
            "inside typeclass \"pointers\"",
            1, 1),

    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance op_ampersand for string {\n"
        "fn dosth(a:i32) -> i32\n"
        "{\n"
        "92821 + a\n"
        "}\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *class_name = testsupport_parser_identifier_create(0, 9, 0, 20);
    struct ast_node *type_name = testsupport_parser_identifier_create(0, 26, 0, 31);

    testsupport_parser_node_create(tinst, typeinstance, 0, 0, 5, 0,
                                   class_name, NULL, type_name, NULL);


    struct ast_node *fn_name = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id2, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typeleaf, 1, 9, 1, 13, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 1, 19, 1, 21);
    testsupport_parser_node_create(fnd, fndecl, 1, 0, 1, 21,
                                   FNDECL_PARTOF_IMPL,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id3
    );

    testsupport_parser_block_create(bnode, 2, 0, 4, 0);
    testsupport_parser_constant_create(cnum1, 3, 0, 3, 4, integer, 92821);
    struct ast_node *id4 = testsupport_parser_identifier_create(3, 8, 3, 8);
    testsupport_parser_node_create(bop1, binaryop, 3, 0, 3, 8,
                                   BINARYOP_ADD, cnum1, id4);
    ast_node_add_child(bnode, bop1);



    testsupport_parser_node_create(fim, fnimpl, 1, 0, 4, 0, fnd, bnode);



    ast_node_add_child(tinst, fim);

    ck_test_parse_as(n, typeinstance, "typeclass instance", tinst);

    ast_node_destroy(n);
    ast_node_destroy(tinst);
}END_TEST

START_TEST(test_acc_typeinstance_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance op_ampersand for string <Type T>{\n"
        "fn dosth<Type T>(a:i32) -> i32\n"
        "{\n"
        "92821 + a\n"
        "}\n"
        "\n"
        "fn do_sth_else(arg:u64)\n"
        "{\n"
        "important_action(arg)\n"
        "}\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *class_name = testsupport_parser_identifier_create(0, 9, 0, 20);
    struct ast_node *type_name = testsupport_parser_identifier_create(0, 26, 0, 31);

    struct ast_node *gid1 = testsupport_parser_identifier_create(0, 34, 0, 37);
    struct ast_node *gid2 = testsupport_parser_identifier_create(0, 39, 0, 39);
    struct ast_node *gtype1 = ast_genrtype_create(gid1, gid2);
    testsupport_parser_node_create(genr1, genrdecl, 0, 33, 0, 40);
    ast_node_add_child(genr1, gtype1);

    testsupport_parser_node_create(tinst, typeinstance, 0, 0, 10, 0,
                                   class_name, NULL, type_name, genr1);


    struct ast_node *fn_name = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *gid3 = testsupport_parser_identifier_create(1, 9, 1, 12);
    struct ast_node *gid4 = testsupport_parser_identifier_create(1, 14, 1, 14);
    struct ast_node *gtype2 = ast_genrtype_create(gid3, gid4);
    testsupport_parser_node_create(genr2, genrdecl, 1, 8, 1, 15);
    ast_node_add_child(genr2, gtype2);

    struct ast_node *id1 = testsupport_parser_identifier_create(1, 17, 1, 17);
    testsupport_parser_xidentifier_create_simple(id2, 1, 19, 1, 21);
    testsupport_parser_node_create(t1, typeleaf, 1, 17, 1, 21, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 1, 27, 1, 29);
    testsupport_parser_node_create(fnd1, fndecl, 1, 0, 1, 29,
                                   FNDECL_PARTOF_IMPL,
                                   fn_name,
                                   genr2,
                                   t1,
                                   id3
    );

    testsupport_parser_block_create(bnode1, 2, 0, 4, 0);
    testsupport_parser_constant_create(cnum1, 3, 0, 3, 4, integer, 92821);
    struct ast_node *id4 = testsupport_parser_identifier_create(3, 8, 3, 8);
    testsupport_parser_node_create(bop1, binaryop, 3, 0, 3, 8,
                                   BINARYOP_ADD, cnum1, id4);
    ast_node_add_child(bnode1, bop1);
    testsupport_parser_node_create(fim1, fnimpl, 1, 0, 4, 0, fnd1, bnode1);
    ast_node_add_child(tinst, fim1);


    struct ast_node *fn_name2 = testsupport_parser_identifier_create(6, 3, 6, 13);
    struct ast_node *id5 = testsupport_parser_identifier_create(6, 15, 6, 17);
    testsupport_parser_xidentifier_create_simple(id6, 6, 19, 6, 21);
    testsupport_parser_node_create(t2, typeleaf, 6, 15, 6, 21, id5, id6);
    testsupport_parser_node_create(fnd2, fndecl, 6, 0, 6, 22,
                                   FNDECL_PARTOF_IMPL,
                                   fn_name2,
                                   NULL,
                                   t2,
                                   NULL
    );

    testsupport_parser_block_create(bnode2, 7, 0, 9, 0);
    struct ast_node *id7 = testsupport_parser_identifier_create(8, 0, 8, 15);
    struct ast_node *id8 = testsupport_parser_identifier_create(8, 17, 8, 19);
    testsupport_parser_node_create(fc1, fncall, 8, 0, 8, 20,
                                   id7, id8, NULL);
    ast_node_add_child(bnode2, fc1);

    testsupport_parser_node_create(fim2, fnimpl, 6, 0, 9, 0, fnd2, bnode2);
    ast_node_add_child(tinst, fim2);


    ck_test_parse_as(n, typeinstance, "typeclass instance", tinst);

    ast_node_destroy(n);
    ast_node_destroy(tinst);
}END_TEST

START_TEST(test_acc_typeinstance_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance op_ampersand nameofinstance for string {\n"
        "fn dosth(a:i32) -> i32\n"
        "{\n"
        "92821 + a\n"
        "}\n"
        "}");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *class_name = testsupport_parser_identifier_create(0, 9, 0, 20);
    struct ast_node *instance_name = testsupport_parser_identifier_create(0, 22, 0, 35);
    struct ast_node *type_name = testsupport_parser_identifier_create(0, 41, 0, 46);

    testsupport_parser_node_create(tinst, typeinstance, 0, 0, 5, 0,
                                   class_name, instance_name, type_name, NULL);


    struct ast_node *fn_name = testsupport_parser_identifier_create(1, 3, 1, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id2, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typeleaf, 1, 9, 1, 13, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 1, 19, 1, 21);
    testsupport_parser_node_create(fnd, fndecl, 1, 0, 1, 21,
                                   FNDECL_PARTOF_IMPL,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id3
    );

    testsupport_parser_block_create(bnode, 2, 0, 4, 0);
    testsupport_parser_constant_create(cnum1, 3, 0, 3, 4, integer, 92821);
    struct ast_node *id4 = testsupport_parser_identifier_create(3, 8, 3, 8);
    testsupport_parser_node_create(bop1, binaryop, 3, 0, 3, 8,
                                   BINARYOP_ADD, cnum1, id4);
    ast_node_add_child(bnode, bop1);



    testsupport_parser_node_create(fim, fnimpl, 1, 0, 4, 0, fnd, bnode);



    ast_node_add_child(tinst, fim);

    ck_test_parse_as(n, typeinstance, "typeclass instance", tinst);

    ast_node_destroy(n);
    ast_node_destroy(tinst);
}END_TEST

START_TEST(test_acc_typeinstance_err1) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the typeclass instance class name "
            "after 'instance'",
            0, 9)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err2) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition && {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected either an identifier or 'for' after the typeinstance "
            "class name",
            0, 18)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err3) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition my_addition * {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a 'for' in the typeclass instance declaration before "
            "the name of the instantiated type",
            0, 30)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err4) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition my_addition for {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the name of the type that 'addition' "
            "typeclass is instantiated with.",
            0, 34)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err5) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition my_addition for vector }");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected '{' at type instance of 'addition' after 'vector'.",
            0, 39)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err6) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition my_addition for vector {}");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected at least one function implementation inside the body of "
            "type instance of 'addition' for 'vector' after '{'.",
            0, 41)
    };
    ck_assert_parser_errors(errors);
} END_TEST

START_TEST(test_acc_typeinstance_err7) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "instance addition my_addition for vector {\n"
        "    fn add self:vector, other:vector) -> u32 {\n"
        "        return 42\n"
        "    }\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(typeinstance);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected '(' at function declaration",
            1, 15),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a proper function implementation inside the body of "
            "type instance of 'addition' for 'vector' after '{'.",
            1, 5),
    };
    ck_assert_parser_errors(errors);
} END_TEST

Suite *parser_typeclass_suite_create(void)
{
    Suite *s = suite_create("parser_typeclass");

    TCase *tpdecl = tcase_create("parser_typeclass_decl_parsing");
    tcase_add_checked_fixture(tpdecl, setup_front_tests, teardown_front_tests);
    tcase_add_test(tpdecl, test_acc_typeclass_1);
    tcase_add_test(tpdecl, test_acc_typeclass_with_generics);
    tcase_add_test(tpdecl, test_acc_typeclass_2);

    TCase *tcerr = tcase_create("parser_typeclass_decl_parsing_errors");
    tcase_add_checked_fixture(tcerr, setup_front_tests, teardown_front_tests);
    tcase_add_test(tcerr, test_acc_typeclass_err1);
    tcase_add_test(tcerr, test_acc_typeclass_err2);
    tcase_add_test(tcerr, test_acc_typeclass_err3);
    tcase_add_test(tcerr, test_acc_typeclass_err4);
    tcase_add_test(tcerr, test_acc_typeclass_err5);
    tcase_add_test(tcerr, test_acc_typeclass_err6);

    TCase *tinst = tcase_create("parser_typeclass_instance_parsing");
    tcase_add_checked_fixture(tinst, setup_front_tests, teardown_front_tests);
    tcase_add_test(tinst, test_acc_typeinstance_1);
    tcase_add_test(tinst, test_acc_typeinstance_2);
    tcase_add_test(tinst, test_acc_typeinstance_3);

    TCase *tinserr = tcase_create("parser_typeclass_instance_errors");
    tcase_add_checked_fixture(tinserr, setup_front_tests, teardown_front_tests);
    tcase_add_test(tinserr, test_acc_typeinstance_err1);
    tcase_add_test(tinserr, test_acc_typeinstance_err2);
    tcase_add_test(tinserr, test_acc_typeinstance_err3);
    tcase_add_test(tinserr, test_acc_typeinstance_err4);
    tcase_add_test(tinserr, test_acc_typeinstance_err5);
    tcase_add_test(tinserr, test_acc_typeinstance_err6);
    tcase_add_test(tinserr, test_acc_typeinstance_err7);


    suite_add_tcase(s, tpdecl);
    suite_add_tcase(s, tcerr);
    suite_add_tcase(s, tinst);
    suite_add_tcase(s, tinserr);
    return s;
}
