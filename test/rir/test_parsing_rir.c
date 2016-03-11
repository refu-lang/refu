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
    struct rir *got_rir;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
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
    );
    front_testdriver_new_rir_source(&s, true);
    ck_create_get_rir(got_rir, 0);

    testsupport_rir_add_module();
    testsupport_rir_add_gstring("false");
    testsupport_rir_add_gstring("foo");
    testsupport_rir_add_gstring("true");

    struct rir_typedef *t1 = testsupport_rir_add_typedef(
        "$internal_struct_4260204557",
        true
    );

    RFS_PUSH();
    struct rir_type *t1_args[] = {
        testsupport_rir_etype("i64", false),
        testsupport_rir_etype("u64", false),
        testsupport_rir_etype("string", false)
    };
    RFS_POP();
    testsupport_rir_typedef_add_arguments(t1, t1_args);

    RFS_PUSH();
    struct rir_type *fn_args[] = {
        testsupport_rir_etype("string", true),
    };
    RFS_POP();
    testsupport_rir_add_fndef(
        "other_function",
        fn_args,
        testsupport_rir_etype("u32", false)
    );
    struct rir_block *b1 = testsupport_rir_add_block("function_start");
    testsupport_rir_block_add_cmd(
        b1,
        convert,
        "$3",
        testsupport_rir_intvalue(14),
        testsupport_rir_etype("u32", false)
    );
    testsupport_rir_block_add_cmd(
        b1,
        write,
        testsupport_rir_value("$1"),
        testsupport_rir_value("$3")
    );

    struct rir_block *b2 = testsupport_rir_add_block("function_end");
    rir_block_exit_init_branch(&b1->exit, &b2->label);
    testsupport_rir_block_add_cmd(
        b2,
        read,
        "$2",
        testsupport_rir_value("$1")
    );
    rir_block_exit_return_init(&b2->exit, testsupport_rir_value("$2"));

    ck_assert_parserir(got_rir);
} END_TEST

Suite *rir_parsing_suite_create(void)
{
    Suite *s = suite_create("rir_parsing");

    TCase *tc1 = tcase_create("simple_rir_parsing");
    tcase_add_checked_fixture(tc1,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(tc1, test_rir_parse1);

    suite_add_tcase(s, tc1);

    return s;
}
