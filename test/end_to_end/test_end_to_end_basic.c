#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>

#include "testsupport_end_to_end.h"

#include CLIB_TEST_HELPERS

START_TEST(test_pipeline_end_to_end) {
    struct end_to_end_driver *d = get_end_to_end_driver();
    static const struct RFstring s = RF_STRING_STATIC_INIT("fn main()->u32{return 42}");
    int expected_ret;
    ck_assert_msg(end_to_end_driver_create_file(d, "test_input_file.rf", &s),
                  "Could not create the input file for the test driver");
    ck_assert_msg(end_to_end_driver_compile(d, "test_input_file.rf"),
                  "Could not compile the input file");
    ck_assert_msg(end_to_end_driver_run(d, &expected_ret),
                  "Failed to execute driver's compiled result");
    ck_assert_int_eq(42, expected_ret);
} END_TEST


Suite *end_to_end_basic_suite_create(void)
{
    Suite *s = suite_create("end_to_end_basic");

    TCase *pipeline = tcase_create("end_to_end_basic_pipeline");
    tcase_add_checked_fixture(pipeline,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(pipeline, test_pipeline_end_to_end);

    suite_add_tcase(s, pipeline);

    return s;
}
