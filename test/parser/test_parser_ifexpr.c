#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/ifexpr.h"
#include <ast/ifexpr.h>
#include <ast/function.h>
#include <ast/operators.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>
#include <ast/arrayref.h>
#include <ast/constant_num.h>
#include <lexer/lexer.h>
#include <info/msg.h>

#include "../testsupport_front.h"
#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_ifexpr_1) {
    struct ast_node *n;
    struct inpfile *file;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "if a == 42 {\n"
        "    do_sth()\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    file = &d->front.file;

    struct ast_node *id1 = testsupport_parser_identifier_create(file,
                                                                0, 3, 0, 3);
    testsupport_parser_constant_create(cnum, file,
                                       0, 8, 0, 9, integer, 42);
    testsupport_parser_node_create(cmp_exp, binaryop, file, 0, 3, 0, 9,
                                   BINARYOP_CMP_EQ, id1, cnum);
    

    testsupport_parser_block_create(bnode, file, 0, 11, 2, 0);
    struct ast_node *fn_name = testsupport_parser_identifier_create(
        file,
        1, 4, 1, 9);

    testsupport_parser_node_create(fc, fncall, file, 1, 4, 1, 11, fn_name, NULL);
    ast_node_add_child(bnode, fc);

    testsupport_parser_node_create(cond, condbranch, file, 0, 3, 2, 0,
                                   cmp_exp, bnode);

    testsupport_parser_node_create(ifx, ifexpr, file, 0, 0, 2, 0, cond, NULL);

    ck_test_parse_as(n, ifexpr, d, "if_expression", ifx);

    ast_node_destroy(n);
    ast_node_destroy(ifx);
}END_TEST


Suite *parser_ifexpr_suite_create(void)
{
    Suite *s = suite_create("if_expression");

    TCase *ifp = tcase_create("if_expression_parsing");
    tcase_add_checked_fixture(ifp, setup_front_tests, teardown_front_tests);
    tcase_add_test(ifp, test_acc_ifexpr_1);

    suite_add_tcase(s, ifp);

    return s;
}
