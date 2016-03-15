#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/rf_str_core.h>
#include <ast/ast.h>

#include "testsupport_end_to_end.h"

#include CLIB_TEST_HELPERS

START_TEST (test_smoke_module_inclusion) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "main.rf",
            "import other\n"
            "fn main()->u32{return 42}"
        ),
        TEST_DECL_SRC(
            "other.rf",

            "module other {\n"
            "}"
        )
    };
    ck_end_to_end_run(inputs, 42);
} END_TEST

Suite *end_to_end_module_suite_create(void)
{
    Suite *s = suite_create("end_to_end_module");

    TCase *st_basic = tcase_create("end_to_end_module_simple");
    tcase_add_checked_fixture(st_basic,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_basic, test_smoke_module_inclusion);
    
    suite_add_tcase(s, st_basic);

    return s;
}
