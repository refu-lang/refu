#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/core.h>
#include <ast/ast.h>
#include <ast/function.h>

#include "testsupport_rir.h"
#include "../testsupport.h"

#include CLIB_TEST_HELPERS

START_TEST (test_rir_parse_single_function) {
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
    testsupport_arr_to_darray(t1->argument_types, t1_args, struct rir_type);

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

START_TEST (test_rir_parse_multiple_functions) {
    struct rir *got_rir;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fndef(foo; i32, i64; u64)"
        "{"
        "%function_start"
        "    $4 = convert($1, i32)"
        "    $5 = cmpgt(i32, $0, $4)"
        "    condbranch($5, %label_1, %label_2)"
        "%label_1"
        "    $6 = convert($1, i32)"
        "    $7 = add(i32, $0, $6)"
        "    $8 = convert($7, u64)"
        "    write(u64*, $2, $8)"
        "    branch(%function_end)"
        "%label_2"
        "    $9 = convert($1, i32)"
        "    $10 = sub(i32, $0, $9)"
        "    $11 = convert($10, u64)"
        "    write(u64*, $2, $11)"
        "    branch(%function_end)"
        "%label_0"
        "    branch(%function_end)"
        "%function_end"
        "    $3 = read($2)"
        "    return($3)"
        "}"
        "fndef(boo; nil; nil)"
        "{"
        "%function_start"
        "    $0 = convert(55, i32)"
        "    $1 = call(foo, defined, $0, 66)"
        "    branch(%function_end)"
        "%function_end"
        "    return()"
        "}"

    );

    front_testdriver_new_rir_source(&s, true);
    ck_create_get_rir(got_rir, 0);

    testsupport_rir_add_module();

    /* -- setting expected data for function foo() -- */

    RFS_PUSH();
    struct rir_type *fn1_args[] = {
        testsupport_rir_etype("i32", false),
        testsupport_rir_etype("i64", false),
    };
    RFS_POP();
    testsupport_rir_add_fndef(
        "foo",
        fn1_args,
        testsupport_rir_etype("u64", false)
    );
    struct rir_block *f1_start = testsupport_rir_add_block("function_start");
    struct rir_block *f1_label1 = testsupport_rir_add_block("label_1");
    struct rir_block *f1_label2 = testsupport_rir_add_block("label_2");
    struct rir_block *f1_label0 = testsupport_rir_add_block("label_0");
    struct rir_block *f1_end = testsupport_rir_add_block("function_end");
    testsupport_rir_block_add_cmd(
        f1_start,
        convert,
        "$4",
        testsupport_rir_value("$1"),
        testsupport_rir_etype("i32", false)
    );
    testsupport_rir_block_add_bop(
        f1_start,
        RIR_EXPRESSION_CMP_GT,
        "$5",
        testsupport_rir_value("$0"),
        testsupport_rir_value("$4")
    );
    rir_block_exit_init_condbranch(
        &f1_start->exit,
        testsupport_rir_value("$5"),
        &f1_label1->label,
        &f1_label2->label
    );

    testsupport_rir_block_add_cmd(
        f1_label1,
        convert,
        "$6",
        testsupport_rir_value("$1"),
        testsupport_rir_etype("i32", false)
    );
    testsupport_rir_block_add_bop(
        f1_label1,
        RIR_EXPRESSION_ADD,
        "$7",
        testsupport_rir_value("$0"),
        testsupport_rir_value("$6")
    );
    testsupport_rir_block_add_cmd(
        f1_label1,
        convert,
        "$8",
        testsupport_rir_value("$7"),
        testsupport_rir_etype("u64", false)
    );
    testsupport_rir_block_add_cmd(
        f1_label1,
        write,
        testsupport_rir_value("$2"),
        testsupport_rir_value("$8")
    );
    rir_block_exit_init_branch(&f1_label1->exit, &f1_end->label);

    testsupport_rir_block_add_cmd(
        f1_label2,
        convert,
        "$9",
        testsupport_rir_value("$1"),
        testsupport_rir_etype("i32", false)
    );
    testsupport_rir_block_add_bop(
        f1_label2,
        RIR_EXPRESSION_SUB,
        "$10",
        testsupport_rir_value("$0"),
        testsupport_rir_value("$0")
    );
    testsupport_rir_block_add_cmd(
        f1_label2,
        convert,
        "$11",
        testsupport_rir_value("$10"),
        testsupport_rir_etype("u64", false)
    );
    testsupport_rir_block_add_cmd(
        f1_label2,
        write,
        testsupport_rir_value("$2"),
        testsupport_rir_value("$11")
    );
    rir_block_exit_init_branch(&f1_label2->exit, &f1_end->label);

    rir_block_exit_init_branch(&f1_label0->exit, &f1_end->label);

    testsupport_rir_block_add_cmd(
        f1_end,
        read,
        "$3",
        testsupport_rir_value("$2")
    );
    rir_block_exit_return_init(&f1_end->exit, testsupport_rir_value("$3"));

    /* -- setting expected data for function boo() -- */

    testsupport_rir_add_fndef(
        "boo",
        NULL,
        NULL
    );
    struct rir_block *f2_start = testsupport_rir_add_block("function_start");
    struct rir_block *f2_end = testsupport_rir_add_block("function_end");
    testsupport_rir_block_add_cmd(
        f2_start,
        convert,
        "$0",
        testsupport_rir_intvalue(55),
        testsupport_rir_etype("i32", false)
    );
    struct rir_value *foo_call_args [] = {
        testsupport_rir_value("$0"),
        testsupport_rir_intvalue(66)
    };
    testsupport_rir_block_add_cmd(
        f2_start,
        call,
        "1",
        "foo",
        false,
        foo_call_args
    );
    rir_block_exit_init_branch(&f2_start->exit, &f2_end->label);
    rir_block_exit_return_init(&f2_end->exit, NULL);


    ck_assert_parserir(got_rir);
} END_TEST

Suite *rir_parsing_suite_create(void)
{
    Suite *s = suite_create("rir_parsing");

    TCase *tc1 = tcase_create("simple_rir_parsing");
    tcase_add_checked_fixture(tc1,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(tc1, test_rir_parse_single_function);
    tcase_add_test(tc1, test_rir_parse_multiple_functions);

    suite_add_tcase(s, tc1);

    return s;
}
