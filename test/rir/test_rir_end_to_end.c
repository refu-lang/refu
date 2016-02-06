#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../end_to_end/testsupport_end_to_end.h"

#include CLIB_TEST_HELPERS

START_TEST (test_rir_end_to_end_simple_module1) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rir",

            "$gstr_3855993015 = global(string, \"false\")\n"
            "$gstr_3784272022 = global(string, \"foo\")\n"
            "$gstr_706834940 = global(string, \"true\")\n"
            "$internal_struct_4260204557 = uniondef(i64, u64, string)\n"
            "fndef(other_function; string*; u32)\n"
            "{\n"
            "%function_start\n"
            "    $3 = convert(14, u32)\n"
            "    write(u32*, $1, $3)\n"
            "    branch(%function_end)\n"
            "%function_end\n"
            "    $2 = read($1)\n"
            "    return($2)\n"
            "}\n"
            "fndef(main; nil; u32)\n"
            "{\n"
            "%function_start\n"
            "    $2 = call(other_function, foreign, $gstr_3784272022)\n"
            "    write(u32*, $0, $2)\n"
            "    branch(%function_end)\n"
            "%function_end\n"
            "    $1 = read($0)\n"
            "    return($1)\n"
            "}\n"
        )
    };
    ck_end_to_end_run(inputs, 14, NULL, "--rir test_input_file.rir");

} END_TEST

Suite *rir_end_to_end_suite_create(void)
{
    Suite *s = suite_create("rir_end_to_end");

    TCase *tc1 = tcase_create("simple_rir_module");
    tcase_add_checked_fixture(tc1,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(tc1, test_rir_end_to_end_simple_module1);

    suite_add_tcase(s, tc1);
    return s;
}
