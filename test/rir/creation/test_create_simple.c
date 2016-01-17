#include "../testsupport_rir.h"

#include CLIB_TEST_HELPERS

START_TEST (test_create_simple_fn) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:u32) -> u64 {\n"
        "return 45 + a\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);
    ck_assert_createrir_ok();

} END_TEST

Suite *rir_creation_simple_suite_create(void)
{
    Suite *s = suite_create("rir_creation_simple");

    TCase *tc1 = tcase_create("test_functions");
    tcase_add_checked_fixture(tc1,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(tc1, test_create_simple_fn);


    suite_add_tcase(s, tc1);

    return s;
}
