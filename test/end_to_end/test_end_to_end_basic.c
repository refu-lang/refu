#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>

#include "testsupport_end_to_end.h"

#include CLIB_TEST_HELPERS

START_TEST(test_smoke) {
    struct end_to_end_driver *d = get_end_to_end_driver();
    static const struct RFstring s = RF_STRING_STATIC_INIT("fn main()->u32{return 42}");
    ck_end_to_end_run(d, "test_input_file.rf", &s, 42);
} END_TEST

START_TEST(test_addition) {
    struct end_to_end_driver *d = get_end_to_end_driver();
    static const struct RFstring s = RF_STRING_STATIC_INIT("fn main()->u32{return 12 + 22}");
    ck_end_to_end_run(d, "test_input_file.rf", &s, 34);
} END_TEST

START_TEST(test_multiple_real_arithmetic) {
    struct end_to_end_driver *d = get_end_to_end_driver();
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn main()->u32{\n"
        "return (12 + (22 * 12 - 33) + (121 * 56 - 29)) / 233\n"
        "}");
    ck_end_to_end_run(d, "test_input_file.rf", &s, 30);
} END_TEST

START_TEST(test_string_literals) {
    struct end_to_end_driver *d = get_end_to_end_driver();
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn main()->u32{\n"
        "string s = \"Celina\""
        "return 13\n"
        "}");
    ck_end_to_end_run(d, "test_input_file.rf", &s, 13);
} END_TEST


Suite *end_to_end_basic_suite_create(void)
{
    Suite *s = suite_create("end_to_end_basic");

    TCase *st_basic = tcase_create("end_to_end_basic");
    tcase_add_checked_fixture(st_basic,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_basic, test_smoke);
    tcase_add_test(st_basic, test_addition);
    tcase_add_test(st_basic, test_multiple_real_arithmetic);
    tcase_add_test(st_basic, test_string_literals);

    suite_add_tcase(s, st_basic);

    return s;
}
