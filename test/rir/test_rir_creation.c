#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS

START_TEST(test_types_list_simple1) {
    struct rir_type *t;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:u64) -> u64 {\n"
        "return 45 + a\n"
        "}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    rir_testdriver_assign(d, &s);
    testsupport_rir_process(d);

    unsigned int i = 0;
    rf_ilist_for_each(&d->rir->rir_types, t, ln) {
        i ++;
    }
    ck_assert_uint_eq(i, 2);
} END_TEST

Suite *rir_creation_suite_create(void)
{
    Suite *s = suite_create("rir_creation");

    TCase *st_basic = tcase_create("rir_creation_types_list");
    tcase_add_checked_fixture(st_basic,
                              setup_rir_tests,
                              teardown_rir_tests);
    tcase_add_test(st_basic, test_types_list_simple1);

    suite_add_tcase(s, st_basic);

    return s;
}
