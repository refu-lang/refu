#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>
#include <ast/function.h>
#include <ast/operators.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_simple_typeclass) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T"
        "}\n"
        "\n"
        "instance addition vector {\n"
        "    fn add(self:vector, other:vector) -> vector\n"
        "    {\n"
        "        ret:vector\n"
        "        ret.x = self.x + other.x\n"
        "        ret.y = self.y + other.y\n"
        "        ret.z = self.z + other.z\n"
        "        return ret\n"
        "    }\n"
        "}\n"
        "fn some_function() {\n"
        "    a:vector = vector(1, 2, 3)\n"
        "    b:vector = vector(3, 2, 1)\n"
        "    c:vector = a.add(b)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    ck_assert_typecheck_ok();
} END_TEST


Suite *analyzer_typecheck_typeclasses_suite_create(void)
{
    Suite *s = suite_create("typecheck_typeclasses");

    TCase *t_typeclasses_simple = tcase_create("typecheck_typeclasses_simple");
    tcase_add_checked_fixture(
        t_typeclasses_simple,
        setup_analyzer_tests,
        teardown_analyzer_tests
    );
    tcase_add_test(t_typeclasses_simple, test_simple_typeclass);


    suite_add_tcase(s, t_typeclasses_simple);

    return s;
}
