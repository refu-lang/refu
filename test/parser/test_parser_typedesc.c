#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <parser/type.h>
#include <ast/ast.h>

#include "testsupport_parser.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_typedesc_simple1) {
    char *sp;
    struct ast_node *n;
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("a:i16");
    struct parser_testdriver *d = get_parser_testdriver();
    int paren_count = 0;
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    sp = parser_file_sp(f);


    struct ast_node *id_1 = ast_identifier_create(f, sp, sp + 1);
    struct ast_node *id_2 = ast_identifier_create(f, sp + 2, sp + 4);
    struct ast_node *type = ast_typedesc_create(f, sp, sp + 4, id_1);
    ast_typedesc_set_right(&type->typedesc, id_2);
    

    n = parser_file_acc_typedesc(f, &paren_count);
    ck_assert_msg(n, "Could not parse type description");
    ck_assert_ast_node_loc(n, 0, 0, 0, 4);
    check_ast_match(n, type);
    
    ast_node_destroy(n);
    ast_node_destroy(type);
}END_TEST



Suite *parser_typedesc_suite_create(void)
{
    Suite *s = suite_create("parser_type_description");

    TCase *td = tcase_create("parset_type_description_simple");
    tcase_add_checked_fixture(td, setup_parser_tests, teardown_parser_tests);
    tcase_add_test(td, test_acc_typedesc_simple1);

    suite_add_tcase(s, td);
    return s;
}
