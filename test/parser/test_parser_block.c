#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/parser.h>
#include "../../src/parser/recursive_descent/block.h"
#include <ast/function.h>
#include <ast/block.h>
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

Suite *parser_block_suite_create(void)
{
    Suite *s = suite_create("parser_block");

    TCase *fp = tcase_create("parser_block_parsing");
    tcase_add_checked_fixture(fp, setup_front_tests, teardown_front_tests);
    tcase_add_test(fp, test_acc_block_empty);


    suite_add_tcase(s, fp);

    return s;
}
