#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/block.h"
#include <ast/function.h>
#include <ast/operators.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/constant_num.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_block_empty) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 1, 0);
    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

START_TEST(test_acc_block_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "a:i32\n"
        "a = 5 + 0.234\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    testsupport_parser_block_create(bnode, file, 0, 0, 3, 0);

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                1, 0, 1, 0);
    testsupport_parser_xidentifier_create_simple(id2, file, 1, 2, 1, 4);
    testsupport_parser_node_create(type1, typedesc, file,
                                   1, 0, 1, 4, id1, id2);
    testsupport_parser_node_create(vardecl, vardecl, file,
                                   1, 0, 1, 4, type1);
    ast_node_add_child(bnode, vardecl);

    struct ast_node *id3 = testsupport_parser_identifier_create(file,
                                                                2, 0, 2, 0);
    testsupport_parser_constant_create(cnum1, file,
                                       2, 4, 2, 4, integer, 5);
    testsupport_parser_constant_create(cnum2, file,
                                       2, 8, 2, 12, float, 0.234);
    testsupport_parser_node_create(op1, binaryop, file, 2, 4, 2, 12,
                                   BINARYOP_ADD, cnum1, cnum2);

    testsupport_parser_node_create(op2, binaryop, file, 2, 0, 2, 12,
                                   BINARYOP_ASSIGN, id3, op1);
    ast_node_add_child(bnode, op2);

    ck_test_parse_as(n, block, d, "block", bnode, true);

    ast_node_destroy(n);
    ast_node_destroy(bnode);
}END_TEST

Suite *parser_block_suite_create(void)
{
    Suite *s = suite_create("parser_block");

    TCase *fp = tcase_create("parser_block_parsing");
    tcase_add_checked_fixture(fp, setup_front_tests, teardown_front_tests);
    tcase_add_test(fp, test_acc_block_empty);
    tcase_add_test(fp, test_acc_block_1);


    suite_add_tcase(s, fp);

    return s;
}
