#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>
#include <ast/function.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS

START_TEST (test_rir_parse1) {

    // TODO: Test rir parsing here

} END_TEST

Suite *rir_parsing_suite_create(void)
{
    Suite *s = suite_create("rir_parsing");

    TCase *tc1 = tcase_create("simple_parsing");
    tcase_add_checked_fixture(tc1,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(tc1, test_rir_parse1);

    suite_add_tcase(s, tc1);

    return s;
}
