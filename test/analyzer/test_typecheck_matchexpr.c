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

#include <types/type.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_typecheck_matchexpr_simple) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i32, s:string}\n"
        "{\n"
        "    a:foo\n"
        "    match a {\n"
        "    i32 => \"number\"\n"
        "    _   => \"other\"\n"
        "    }\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert_typecheck_ok(d, true);
} END_TEST


Suite *analyzer_typecheck_matchexpr_suite_create(void)
{
    Suite *s = suite_create("typecheck_match_expressions");

    TCase *t_simple = tcase_create("simple_matchexpr");
    tcase_add_checked_fixture(t_simple,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(t_simple, test_typecheck_matchexpr_simple);

    suite_add_tcase(s, t_simple);
    return s;
}


