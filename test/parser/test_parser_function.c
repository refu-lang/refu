#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/function.h"
#include <ast/function.h>
#include <ast/type.h>
#include <ast/string_literal.h>
#include <ast/constants.h>
#include <ast/generics.h>
#include <ast/block.h>
#include <ast/matchexpr.h>
#include <ast/operators.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_fndecl_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:i32) -> i32");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 9, 0, 9);
    testsupport_parser_xidentifier_create_simple(id2, 0, 11, 0, 13);
    testsupport_parser_node_create(t1, typeleaf, 0, 9, 0, 13, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, 0, 19, 0, 21);
    testsupport_parser_node_create(fn, fndecl, 0, 0, 0, 21,
                                   FNDECL_STANDALONE,
                                   name,
                                   NULL,
                                   t1,
                                   id3
    );

    ck_test_parse_as(n, fndecl, "function", fn, FNDECL_STANDALONE);

    ast_node_destroy(n);
    ast_node_destroy(fn);
}END_TEST

START_TEST(test_acc_fndecl_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:i32, b:string) -> i32|nil");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 9, 0, 9);
    testsupport_parser_xidentifier_create_simple(id2, 0, 11, 0, 13);
    testsupport_parser_node_create(t1, typeleaf, 0, 9, 0, 13, id1, id2);
    struct ast_node *id3 = testsupport_parser_identifier_create(0, 16, 0, 16);
    testsupport_parser_xidentifier_create_simple(id4, 0, 18, 0, 23);
    testsupport_parser_node_create(t2, typeleaf, 0, 16, 0, 23, id3, id4);
    testsupport_parser_node_create(op1, typeop, 0, 9, 0, 23,
                                   TYPEOP_PRODUCT, t1, t2);

    testsupport_parser_xidentifier_create_simple(id5, 0, 29, 0, 31);
    testsupport_parser_xidentifier_create_simple(id6, 0, 33, 0, 35);
    testsupport_parser_node_create(op2, typeop, 0, 29, 0, 35,
                                   TYPEOP_SUM, id5, id6);

    testsupport_parser_node_create(fn, fndecl, 0, 0, 0, 35,
                                   FNDECL_STANDALONE,
                                   name,
                                   NULL,
                                   op1,
                                   op2
    );

    ck_test_parse_as(n, fndecl, "function", fn, FNDECL_STANDALONE);

    ast_node_destroy(n);
    ast_node_destroy(fn);
}END_TEST

START_TEST(test_acc_fndecl_void) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth_no_args()");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 15);

    testsupport_parser_node_create(fn, fndecl, 0, 0, 0, 17,
                                   FNDECL_STANDALONE,
                                   name,
                                   NULL,
                                   NULL,
                                   NULL
    );

    ck_test_parse_as(n, fndecl, "function", fn, FNDECL_STANDALONE);

    ast_node_destroy(n);
    ast_node_destroy(fn);
}END_TEST

START_TEST(test_acc_fndecl_with_generics) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_generic<Type a, Type b>(a:b, x:string) -> (r1:i32,r2:i8)");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 12);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 14, 0, 17);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 19, 0, 19);
    struct ast_node *gtype1 = ast_genrtype_create(id1, id2);
    struct ast_node *id3 = testsupport_parser_identifier_create(0, 22, 0, 25);
    struct ast_node *id4 = testsupport_parser_identifier_create(0, 27, 0, 27);
    struct ast_node *gtype2 = ast_genrtype_create(id3, id4);
    testsupport_parser_node_create(genr, genrdecl, 0, 13, 0, 28);
    ast_node_add_child(genr, gtype1);
    ast_node_add_child(genr, gtype2);


    struct ast_node *id5 = testsupport_parser_identifier_create(0, 30, 0, 30);
    testsupport_parser_xidentifier_create_simple(id6, 0, 32, 0, 32);
    testsupport_parser_node_create(t1, typeleaf, 0, 30, 0, 32, id5, id6);
    struct ast_node *id7 = testsupport_parser_identifier_create(0, 35, 0, 35);
    testsupport_parser_xidentifier_create_simple(id8, 0, 37, 0, 42);
    testsupport_parser_node_create(t2, typeleaf, 0, 35, 0, 42, id7, id8);
    testsupport_parser_node_create(op1, typeop, 0, 30, 0, 42,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id9 = testsupport_parser_identifier_create(0, 49, 0, 50);
    testsupport_parser_xidentifier_create_simple(id10, 0, 52, 0, 54);
    testsupport_parser_node_create(t3, typeleaf, 0, 49, 0, 54, id9, id10);
    struct ast_node *id11 = testsupport_parser_identifier_create(0, 56, 0, 57);
    testsupport_parser_xidentifier_create_simple(id12, 0, 59, 0, 60);
    testsupport_parser_node_create(t4, typeleaf, 0, 56, 0, 60, id11, id12);
    testsupport_parser_typedesc_create(op2, 0, 49, 0, 60, typeop,
                                       TYPEOP_PRODUCT, t3, t4);

    testsupport_parser_node_create(fn, fndecl, 0, 0, 0, 60,
                                   FNDECL_STANDALONE,
                                   name,
                                   genr,
                                   op1,
                                   op2
    );

    ck_test_parse_as(n, fndecl, "function", fn, FNDECL_STANDALONE);

    ast_node_destroy(n);
    ast_node_destroy(fn);
}END_TEST

START_TEST(test_acc_fndecl_err1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn ()");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected an identifier for the function name after 'fn'",
            0, 3)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fndecl_err2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth)");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected '(' at function declaration",
            0, 8)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fndecl_err3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:int");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected ')' at function declaration after type description",
            0, 13)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fndecl_err4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:int, )");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a '(' or identifier after ','",
            0, 14),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected either a type description for the function's arguments "
            "or ')' after '('",
            0, 8),
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fndecl_err5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:int) ->");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected type description for the function's return type after"
            " '->'",
            0, 17)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fndecl_err6) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn dosth(a:int) -> (a:int, ");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fndecl, FNDECL_STANDALONE);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a '(' or identifier after ','",
            0, 25),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a type description after '('",
            0, 19),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected type description for the function's return type after"
            " '->'",
            0, 17)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fncall_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "foo(a, b)");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 0, 0, 2);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 4, 0, 4);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 7, 0, 7);

    testsupport_parser_node_create(args, binaryop, 0, 4, 0, 7,
                                   BINARYOP_COMMA,
                                   id1, id2);
    testsupport_parser_node_create(fc, fncall, 0, 0, 0, 8,
                                   name,
                                   args,
                                   NULL
    );
    ck_test_parse_as(n, fncall, "function_call", fc, true);

    ast_node_destroy(n);
    ast_node_destroy(fc);
}END_TEST

START_TEST(test_acc_fncall_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "do_something (a, b, 31, \"celka\")");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 0, 0, 11);
    struct ast_node *id1 = testsupport_parser_identifier_create(0, 14, 0, 14);
    struct ast_node *id2 = testsupport_parser_identifier_create(0, 17, 0, 17);
    testsupport_parser_constant_create(cnum,
                                       0, 20, 0, 21, integer, 31);
    testsupport_parser_string_literal_create(sliteral,
                                             0, 24, 0, 30);

    testsupport_parser_node_create(bop1, binaryop, 0, 14, 0, 17,
                                   BINARYOP_COMMA,
                                   id1, id2);
    testsupport_parser_node_create(bop2, binaryop, 0, 14, 0, 21,
                                   BINARYOP_COMMA,
                                   bop1, cnum);
    testsupport_parser_node_create(args, binaryop, 0, 14, 0, 30,
                                   BINARYOP_COMMA,
                                   bop2, sliteral);
    testsupport_parser_node_create(fc, fncall, 0, 0, 0, 31,
                                   name,
                                   args,
                                   NULL
    );

    ck_test_parse_as(n, fncall, "function_call", fc, true);

    ast_node_destroy(n);
    ast_node_destroy(fc);
}END_TEST

START_TEST(test_acc_fncall_3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "do_something <a, b> (a, b, 31, \"celka\")");
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 0, 0, 11);
    testsupport_parser_xidentifier_create_simple(id1,
                                                 0, 14, 0, 14);
    testsupport_parser_xidentifier_create_simple(id2,
                                                 0, 17, 0, 17);

    testsupport_parser_node_create(gnattr, genrattr, 0, 13, 0, 18);
    ast_node_add_child(gnattr, id1);
    ast_node_add_child(gnattr, id2);

    struct ast_node *id3 = testsupport_parser_identifier_create(0, 21, 0, 21);
    struct ast_node *id4 = testsupport_parser_identifier_create(0, 24, 0, 24);
    testsupport_parser_constant_create(cnum,
                                       0, 27, 0, 28, integer, 31);
    testsupport_parser_string_literal_create(sliteral,
                                             0, 31, 0, 37);

    testsupport_parser_node_create(bop1, binaryop, 0, 21, 0, 24,
                                   BINARYOP_COMMA,
                                   id3, id4);
    testsupport_parser_node_create(bop2, binaryop, 0, 21, 0, 28,
                                   BINARYOP_COMMA,
                                   bop1, cnum);
    testsupport_parser_node_create(args, binaryop, 0, 21, 0, 37,
                                   BINARYOP_COMMA,
                                   bop2, sliteral);
    testsupport_parser_node_create(fc, fncall, 0, 0, 0, 38,
                                   name,
                                   args,
                                   gnattr);

    ck_test_parse_as(n, fncall, "function_call", fc, true);

    ast_node_destroy(n);
    ast_node_destroy(fc);
}END_TEST

START_TEST(test_acc_fncall_err1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "foo(");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fncall, true);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected ')' at end of foo function call",
            0, 3)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fncall_err2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "foo(a,");
    front_testdriver_new_ast_main_source(&s);

    ck_test_fail_parse_as(fncall, true);
    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected a string literal, a numeric constant or an identifier after \",\"",
            0, 5),
        TESTSUPPORT_INFOMSG_INIT_START(
            MESSAGE_SYNTAX_ERROR,
            "Expected argument expression for function call",
            0, 5)
    };
    ck_assert_parser_errors(errors);
}END_TEST

START_TEST(test_acc_fnimpl_1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_awesome_stuff(a:u32, f:f64 | s:string)\n"
        "{\n"
        "f = 3.214 * a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 18);

    struct ast_node *id1 = testsupport_parser_identifier_create(0, 20, 0, 20);
    testsupport_parser_xidentifier_create_simple(id2,
                                                 0, 22, 0, 24);
    testsupport_parser_node_create(t1, typeleaf,
                                   0, 20, 0, 24, id1, id2);

    struct ast_node *id3 = testsupport_parser_identifier_create(0, 27, 0, 27);
    testsupport_parser_xidentifier_create_simple(id4,
                                                 0, 29, 0, 31);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, 27, 0, 31, id3, id4);

    testsupport_parser_node_create(op1, typeop, 0, 20, 0, 31,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id5 = testsupport_parser_identifier_create(0, 35, 0, 35);
    testsupport_parser_xidentifier_create_simple(id6, 0, 37, 0, 42);
    testsupport_parser_node_create(t3, typeleaf,
                                   0, 35, 0, 42, id5, id6);

    testsupport_parser_node_create(op2, typeop, 0, 20, 0, 42,
                                   TYPEOP_SUM, op1, t3);

    testsupport_parser_node_create(decl, fndecl, 0, 0, 0, 43,
                                   FNDECL_PARTOF_IMPL, name, NULL, op2, NULL);


    testsupport_parser_block_create(bnode, 1, 0, 3, 0);
    struct ast_node *id7 = testsupport_parser_identifier_create(2, 0, 2, 0);
    testsupport_parser_constant_create(cnum,
                                       2, 4, 2, 8, float, 3.214);
    struct ast_node *id8 = testsupport_parser_identifier_create(2, 12, 2, 12);

    testsupport_parser_node_create(bop1, binaryop, 2, 4, 2, 12,
                                   BINARYOP_MUL, cnum, id8);
    testsupport_parser_node_create(bop, binaryop, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id7, bop1);
    ast_node_add_child(bnode, bop);


    testsupport_parser_node_create(fim, fnimpl,
                                   0, 0, 3, 0, decl, bnode);

    ck_test_parse_as(n, fnimpl, "function_implementation", fim);

    ast_node_destroy(n);
    ast_node_destroy(fim);
}END_TEST

START_TEST(test_acc_fnimpl_2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_awesome_stuff<Type T>(a:u32, f:f64 | s:string)\n"
        "{\n"
        "f = 3.214 * a\n"
        "s[3] = 33\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 18);

    struct ast_node *gid1 = testsupport_parser_identifier_create(0, 20, 0, 23);
    struct ast_node *gid2 = testsupport_parser_identifier_create(0, 25, 0, 25);
    struct ast_node *gtype1 = ast_genrtype_create(gid1, gid2);
    testsupport_parser_node_create(genr, genrdecl, 0, 19, 0, 26);
    ast_node_add_child(genr, gtype1);


    struct ast_node *id1 = testsupport_parser_identifier_create(0, 28, 0, 28);
    testsupport_parser_xidentifier_create_simple(id2,
                                                 0, 30, 0, 32);
    testsupport_parser_node_create(t1, typeleaf,
                                   0, 28, 0, 32, id1, id2);

    struct ast_node *id3 = testsupport_parser_identifier_create(0, 35, 0, 35);
    testsupport_parser_xidentifier_create_simple(id4,
                                                 0, 37, 0, 39);
    testsupport_parser_node_create(t2, typeleaf,
                                   0, 35, 0, 39, id3, id4);

    testsupport_parser_node_create(op1, typeop, 0, 28, 0, 39,
                                   TYPEOP_PRODUCT, t1, t2);

    struct ast_node *id5 = testsupport_parser_identifier_create(0, 43, 0, 43);
    testsupport_parser_xidentifier_create_simple(id6,
                                                 0, 45, 0, 50);
    testsupport_parser_node_create(t3, typeleaf,
                                   0, 43, 0, 50, id5, id6);

    testsupport_parser_node_create(op2, typeop, 0, 28, 0, 50,
                                   TYPEOP_SUM, op1, t3);

    testsupport_parser_node_create(decl, fndecl, 0, 0, 0, 51,
                                   FNDECL_PARTOF_IMPL, name, genr, op2, NULL);


    testsupport_parser_block_create(bnode, 1, 0, 4, 0);
    struct ast_node *id7 = testsupport_parser_identifier_create(2, 0, 2, 0);
    testsupport_parser_constant_create(cnum,
                                       2, 4, 2, 8, float, 3.214);
    struct ast_node *id8 = testsupport_parser_identifier_create(2, 12, 2, 12);

    testsupport_parser_node_create(bop1, binaryop, 2, 4, 2, 12,
                                   BINARYOP_MUL, cnum, id8);
    testsupport_parser_node_create(bop2, binaryop, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id7, bop1);
    ast_node_add_child(bnode, bop2);


    struct ast_node *id9 = testsupport_parser_identifier_create(3, 0, 3, 0);
    testsupport_parser_constant_create(cnum2,
                                       3, 2, 3, 2, integer, 3);
    testsupport_parser_constant_create(cnum3,
                                       3, 7, 3, 8, integer, 33);
    testsupport_parser_node_create(arr, binaryop, 3, 0, 3, 3,
                                   BINARYOP_ARRAY_REFERENCE, id9, cnum2);
    testsupport_parser_node_create(bop3, binaryop, 3, 0, 3, 8,
                                   BINARYOP_ASSIGN, arr, cnum3);
    ast_node_add_child(bnode, bop3);

    testsupport_parser_node_create(fim, fnimpl,
                                   0, 0, 4, 0, decl, bnode);

    ck_test_parse_as(n, fnimpl, "function_implementation", fim);

    ast_node_destroy(n);
    ast_node_destroy(fim);
} END_TEST

START_TEST (test_acc_fnimpl_no_braces) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn action(a:i32 | b:string)\n"
        "a:i32    => print(a)\n"
        "b:string => print(b)\n"
        "\n"
    );
    front_testdriver_new_ast_main_source(&s);

    struct ast_node *name = testsupport_parser_identifier_create(0, 3, 0, 8);
    struct ast_node *id_a = testsupport_parser_identifier_create(0, 10, 0, 10);
    testsupport_parser_xidentifier_create_simple(id_i32, 0, 12, 0, 14);
    testsupport_parser_node_create(t1, typeleaf, 0, 10, 0, 14, id_a, id_i32);

    struct ast_node *id_b = testsupport_parser_identifier_create(0, 18, 0, 18);
    testsupport_parser_xidentifier_create_simple(id_string, 0, 20, 0, 25);
    testsupport_parser_node_create(t2, typeleaf, 0, 18, 0, 25, id_b, id_string);
    testsupport_parser_node_create(fn_arg_type, typeop, 0, 10, 0, 25,
                                   TYPEOP_SUM, t1, t2);
    testsupport_parser_node_create(decl, fndecl, 0, 0, 0, 26,
                                   FNDECL_PARTOF_IMPL, name, NULL, fn_arg_type, NULL);
    

    // match expression
    // case 1
    struct ast_node *id_ma = testsupport_parser_identifier_create(1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id_mi32, 1, 2, 1, 4);
    testsupport_parser_typedesc_create(t_mai32, 1, 0, 1, 4, typeleaf, id_ma, id_mi32);
    struct ast_node *id_print1 = testsupport_parser_identifier_create(1, 12, 1, 16);
    struct ast_node *id_pa = testsupport_parser_identifier_create(1, 18, 1, 18);
    testsupport_parser_node_create(fc1, fncall, 1, 12, 1, 19,
                                   id_print1,
                                   id_pa,
                                   NULL);
    testsupport_parser_node_create(mcase1, matchcase, 1, 0, 1, 19,
                                   t_mai32, fc1);
    // case 2
    struct ast_node *id_mb = testsupport_parser_identifier_create(2, 0, 2, 0);
    testsupport_parser_xidentifier_create_simple(id_mstring, 2, 2, 2, 7);
    testsupport_parser_typedesc_create(t_mstring, 2, 0, 2, 7, typeleaf, id_mb, id_mstring);
    struct ast_node *id_print2 = testsupport_parser_identifier_create(2, 12, 2, 16);
    struct ast_node *id_pb = testsupport_parser_identifier_create(2, 18, 2, 18);
    testsupport_parser_node_create(fc2, fncall, 2, 12, 2, 19,
                                   id_print2,
                                   id_pb,
                                   NULL);
    testsupport_parser_node_create(mcase2, matchcase, 2, 0, 2, 19,
                                   t_mstring, fc2);
    // add the cases
    testsupport_parser_node_create(mexpr, matchexpr, 1, 0, 2, 19, NULL);
    ast_node_add_child(mexpr, mcase1);
    ast_node_add_child(mexpr, mcase2);
    // finally form the function impl
    testsupport_parser_node_create(fim, fnimpl,
                                   0, 0, 2, 19, decl, mexpr);

    ck_test_parse_as(n, fnimpl, "function_implementation", fim);

    ast_node_destroy(n);
    ast_node_destroy(fim);
} END_TEST

Suite *parser_function_suite_create(void)
{
    Suite *s = suite_create("parser_function");

    TCase *fp = tcase_create("parser_functiondecl_parsing");
    tcase_add_checked_fixture(fp, setup_front_tests, teardown_front_tests);
    tcase_add_test(fp, test_acc_fndecl_1);
    tcase_add_test(fp, test_acc_fndecl_2);
    tcase_add_test(fp, test_acc_fndecl_void);
    tcase_add_test(fp, test_acc_fndecl_with_generics);

    TCase *fpf = tcase_create("parser_functiondecl_parsing_failures");
    tcase_add_checked_fixture(fpf, setup_front_tests, teardown_front_tests);
    tcase_add_test(fpf, test_acc_fndecl_err1);
    tcase_add_test(fpf, test_acc_fndecl_err2);
    tcase_add_test(fpf, test_acc_fndecl_err3);
    tcase_add_test(fpf, test_acc_fndecl_err4);
    tcase_add_test(fpf, test_acc_fndecl_err5);
    tcase_add_test(fpf, test_acc_fndecl_err6);

    TCase *fcall = tcase_create("parser_function_call");
    tcase_add_checked_fixture(fcall, setup_front_tests, teardown_front_tests);
    tcase_add_test(fcall, test_acc_fncall_1);
    tcase_add_test(fcall, test_acc_fncall_2);
    tcase_add_test(fcall, test_acc_fncall_3);

    TCase *fcall_f = tcase_create("parser_function_call_failures");
    tcase_add_checked_fixture(fcall_f, setup_front_tests, teardown_front_tests);
    tcase_add_test(fcall_f, test_acc_fncall_err1);
    tcase_add_test(fcall_f, test_acc_fncall_err2);

    TCase *fimpl = tcase_create("parser_function_implementation");
    tcase_add_checked_fixture(fimpl, setup_front_tests, teardown_front_tests);
    tcase_add_test(fimpl, test_acc_fnimpl_1);
    tcase_add_test(fimpl, test_acc_fnimpl_2);
    tcase_add_test(fimpl, test_acc_fnimpl_no_braces);



    suite_add_tcase(s, fp);
    suite_add_tcase(s, fpf);
    suite_add_tcase(s, fcall);
    suite_add_tcase(s, fcall_f);
    suite_add_tcase(s, fimpl);
    return s;
}
