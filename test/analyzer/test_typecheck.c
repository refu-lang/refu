#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <info/msg.h>
#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include <ast/ast.h>
#include <ast/block.h>
#include <ast/identifier.h>
#include <ast/function.h>
#include <ast/vardecl.h>
#include <ast/type.h>

#include <types/type.h>

#include "../../src/analyzer/analyzer_pass1.h"

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

/* -- simple symbol table functionality tests -- */

START_TEST(test_typecheck_assignment_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "name:string\n"
        "b:f64\n"
        "a = 456\n"
        "name = \"Lefteris\"\n"
        "b = 0.231\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

START_TEST(test_typecheck_assignment_conversion) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u8\n"
        "b:f32\n"
        "a = 456\n"
        "b = 0.231\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            &d->front.file,
            MESSAGE_SEMANTIC_WARNING,
            "Assignment from a larger to a smaller builtin type."
            "\"u64\" to \"u8\"",
            2, 0, 2, 6)
    };

    // set conversion warnings on
    front_ctx_set_warn_on_implicit_conversions(&d->front, true);

    ck_assert_typecheck_with_messages(d, true, messages);
} END_TEST


START_TEST(test_typecheck_addition_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{a:u64\n"
        "b:u64\n"
        "c:u64\n"
        "a = b + c\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_typecheck_prepare(d);
    ck_assert_typecheck_ok(d);
} END_TEST

Suite *analyzer_typecheck_suite_create(void)
{
    Suite *s = suite_create("analyzer_type_check");

    TCase *st1 = tcase_create("analyzer_typecheck_assignments");
    tcase_add_checked_fixture(st1,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st1, test_typecheck_assignment_simple);
    tcase_add_test(st1, test_typecheck_assignment_conversion);

    TCase *st2 = tcase_create("analyzer_typecheck_additions");
    tcase_add_checked_fixture(st2,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st2, test_typecheck_addition_simple);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}


