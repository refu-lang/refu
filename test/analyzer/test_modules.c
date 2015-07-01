#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>

#include <ast/function.h>

#include "../testsupport_front.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST (test_single_dependency) {
    static const struct RFstring a = RF_STRING_STATIC_INIT(
        "module a { \n"
        "import b\n"
        "}\n"
    );
    static const struct RFstring b = RF_STRING_STATIC_INIT(
        "module b {}\n"
    );
    front_testdriver_new_source(&a);
    front_testdriver_new_source(&b);

    static const struct RFstring expected_modules[] = {
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("a")
    };
    ck_assert_modules_order(expected_modules);

} END_TEST


Suite *analyzer_modules_suite_create(void)
{
    Suite *s = suite_create("analyzer_modules");

    TCase *t_1 = tcase_create("modules_dependency_order");
    tcase_add_checked_fixture(t_1,
                              setup_analyzer_tests_no_stdlib,
                              teardown_analyzer_tests);
    tcase_add_test(t_1, test_single_dependency);


    suite_add_tcase(s, t_1);
    return s;
}
