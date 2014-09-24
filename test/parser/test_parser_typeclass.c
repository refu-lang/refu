#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/typeclass.h"
#include <ast/function.h>
#include <ast/type.h>
#include <ast/typeclass.h>
#include <ast/generics.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typeclass_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand {\n"
        "fn dosth(a:i32) -> i32\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    struct ast_node *name = testsupport_parser_identifier_create(file,
                                                                0, 6, 0, 17);
    testsupport_parser_node_create(tclass, typeclass, file, 0, 0, 2, 0,
                                   name,
                                   NULL);
    struct ast_node *fn_name = testsupport_parser_identifier_create(file,
                                                                    1, 3, 1, 7);
    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typedesc, file, 1, 9, 1, 13, id1, id2);
    testsupport_parser_xidentifier_create_simple(id3, file, 1, 19, 1, 21);
    testsupport_parser_node_create(fn, fndecl, file, 1, 0, 1, 21,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id3
    );
    ast_node_add_child(tclass, fn);

    ck_test_parse_as(n, typeclass, d, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_with_generics) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand <Type foo> {\n"
        "fn dosth(a:foo) -> i32\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    struct ast_node *name = testsupport_parser_identifier_create(file,
                                                                 0, 6, 0, 17);
    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 20, 0, 23);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 25, 0, 27);
    struct ast_node *gtype = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, file, 0, 19, 0, 28);
    ast_node_add_child(genr, gtype);

    testsupport_parser_node_create(tclass, typeclass, file, 0, 0, 2, 0,
                                   name,
                                   genr);

    struct ast_node *fn_name = testsupport_parser_identifier_create(file,
                                                                    1, 3, 1, 7);
    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id4, file, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typedesc, file, 1, 9, 1, 13, id3, id4);
    testsupport_parser_xidentifier_create_simple(id5, file, 1, 19, 1, 21);
    testsupport_parser_node_create(fn, fndecl, file, 1, 0, 1, 21,
                                   fn_name,
                                   NULL,
                                   t1,
                                   id5
    );
    ast_node_add_child(tclass, fn);

    ck_test_parse_as(n, typeclass, d, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_2) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class op_ampersand <Type foo> {\n"
        "fn dosth(a:foo) -> i32\n"
        "fn dosth_else() -> i32\n"
        "fn act(a:u64) -> foo\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    struct ast_node *name = testsupport_parser_identifier_create(file,
                                                                 0, 6, 0, 17);
    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 20, 0, 23);
    struct ast_node *id2 = testsupport_parser_identifier_create(file,
                                                                0, 25, 0, 27);
    struct ast_node *gtype = ast_genrtype_create(id1, id2);
    testsupport_parser_node_create(genr, genrdecl, file, 0, 19, 0, 28);
    ast_node_add_child(genr, gtype);

    testsupport_parser_node_create(tclass, typeclass, file, 0, 0, 4, 0,
                                   name,
                                   genr);

    struct ast_node *fn_name1 = testsupport_parser_identifier_create(file,
                                                                    1, 3, 1, 7);
    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                1, 9, 1, 9);
    testsupport_parser_xidentifier_create_simple(id4, file, 1, 11, 1, 13);
    testsupport_parser_node_create(t1, typedesc, file, 1, 9, 1, 13, id3, id4);
    testsupport_parser_xidentifier_create_simple(id5, file, 1, 19, 1, 21);
    testsupport_parser_node_create(fn1, fndecl, file, 1, 0, 1, 21,
                                   fn_name1,
                                   NULL,
                                   t1,
                                   id5
    );
    ast_node_add_child(tclass, fn1);

    struct ast_node *fn_name2 = testsupport_parser_identifier_create(file,
                                                                    2, 3, 2, 12);
    testsupport_parser_xidentifier_create_simple(id6, file, 2, 19, 2, 21);
    testsupport_parser_node_create(fn2, fndecl, file, 2, 0, 2, 21,
                                   fn_name2,
                                   NULL,
                                   NULL,
                                   id6
    );
    ast_node_add_child(tclass, fn2);

    struct ast_node *fn_name3 = testsupport_parser_identifier_create(file,
                                                                    3, 3, 3, 5);
    struct ast_node *id7 = testsupport_parser_identifier_create(file,
                                                                3, 7, 3, 7);
    testsupport_parser_xidentifier_create_simple(id8, file, 3, 9, 3, 11);
    testsupport_parser_node_create(t2, typedesc, file, 3, 7, 3, 11, id7, id8);
    testsupport_parser_xidentifier_create_simple(id9, file, 3, 17, 3, 19);
    testsupport_parser_node_create(fn3, fndecl, file, 3, 0, 3, 19,
                                   fn_name3,
                                   NULL,
                                   t2,
                                   id9
    );
    ast_node_add_child(tclass, fn3);

    ck_test_parse_as(n, typeclass, d, "typeclass", tclass);

    ast_node_destroy(n);
    ast_node_destroy(tclass);
}END_TEST

START_TEST(test_acc_typeclass_err1) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class {}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected an identifier for the typeclass name after 'class'",
            0, 6)
    };
    ck_assert_parser_errors(d->front.info, errors);
} END_TEST

START_TEST(test_acc_typeclass_err2) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected at least one function declaration inside the body of "
            "typeclass \"pointers\" after '{'",
            0, 15)
    };
    ck_assert_parser_errors(d->front.info, errors);
} END_TEST

START_TEST(test_acc_typeclass_err3) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class adder <Type a {}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected either a ',' or a '>' at generic declaration",
            0, 20),
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected a generic declaration for typeclass \"adder\" "
            "after identifier",
            0, 10)
    };
    ck_assert_parser_errors(d->front.info, errors);
} END_TEST

START_TEST(test_acc_typeclass_err4) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers \n"
        "fn do_sth()\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected '{' at \"pointers\" typeclass "
            "declaration after identifier",
            0, 13)
    };
    ck_assert_parser_errors(d->front.info, errors);
} END_TEST

START_TEST(test_acc_typeclass_err5) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn do_sth()");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected '}' at the end of \"pointers\" typeclass declaration",
            1, 10)
    };
    ck_assert_parser_errors(d->front.info, errors);
} END_TEST

START_TEST(test_acc_typeclass_err6) {
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn dosth(\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(lexer_scan(d->front.lexer));
    n = parser_acc_typeclass(d->front.parser);
    ck_assert_msg(n == NULL, "parsing typeclass should fail");
    ck_assert_msg(
        parser_has_syntax_error(d->front.parser),
        "a syntax error should have been reported");

    struct info_msg errors[] = {
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected ')' at function declaration after '('",
            1, 8),
        TESTPARSER_MSG_INIT_START(
            &d->front.file,
            "Expected a proper function declaration "
            "inside typeclass \"pointers\"",
            2, 0),

    };
    ck_assert_parser_errors(d->front.info, errors);
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

    suite_add_tcase(s, tpdecl);
    suite_add_tcase(s, tcerr);
    return s;
}
