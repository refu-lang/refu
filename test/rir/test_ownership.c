#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/rf_str_core.h>
#include <ast/ast.h>
#include <ast/function.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS

START_TEST (test_usage_1) {
    // TODO: This test is still in progress
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person {name:string, age:u32}\n"
        "fn puse(p:person)\n"
        "{\n"
        "    print(p.name)\n"
        "}\n"
        "fn main() -> u32{\n"
        "    p:person = person(\"Lef\", 29)\n"
        "    puse(p)\n"
        "    print(p.name)\n"
        "    return 22\n"
        "}\n"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_createrir_ok();
} END_TEST

Suite *ownership_suite_create(void)
{
    Suite *s = suite_create("ownership");

    TCase *tc1 = tcase_create("ownership_usage");
    tcase_add_checked_fixture(tc1,
                              setup_rir_tests,
                              teardown_rir_tests);
    tcase_add_test(tc1, test_usage_1);

    suite_add_tcase(s, tc1);

    return s;
}
