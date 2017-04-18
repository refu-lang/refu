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
        "    x:int, y:int, z:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T"
        "}\n"
        "\n"
        "instance addition for vector {\n"
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

START_TEST(test_typeclass_2_instances_diff_types) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "type person {\n"
        "    name:string, age:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T"
        "}\n"
        "\n"
        "instance addition for vector {\n"
        "    fn add(self:vector, other:vector) -> vector\n"
        "    {\n"
        "        ret:vector\n"
        "        ret.x = self.x + other.x\n"
        "        ret.y = self.y + other.y\n"
        "        ret.z = self.z + other.z\n"
        "        return ret\n"
        "    }\n"
        "}\n"
        "instance addition for person {\n"
        "    fn add(self:person, other:person) ->person\n"
        "    {\n"
        "        ret:person\n"
        "        ret.age = self.age + other.age\n"
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

START_TEST(test_typeclass_2_instances_same_type) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T"
        "}\n"
        "\n"
        "instance addition normal for vector {\n"
        "    fn add(self:vector, other:vector) -> vector\n"
        "    {\n"
        "        ret:vector\n"
        "        ret.x = self.x + other.x\n"
        "        ret.y = self.y + other.y\n"
        "        ret.z = self.z + other.z\n"
        "        return ret\n"
        "    }\n"
        "}\n"
        "instance addition double for vector {\n"
        "    fn add(self:vector, other:vector) ->vector\n"
        "    {\n"
        "        ret:vector\n"
        "        ret.x = (self.x + other.x) * 2\n"
        "        ret.y = (self.y + other.y) * 2\n"
        "        ret.z = (self.z + other.z) * 2\n"
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

START_TEST(test_typeclass_errors1) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T\n"
        "}\n"
        "\n"
        "instance addition for vector {\n"
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
        "    c:vector = a.add(1)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "function add() is called with argument type of "
            "\"vector,u8\" which does not match the expected type of "
            "\"vector,vector\".",
            20, 17, 20, 22)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typeclass_errors2) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(self:T, other:T) -> T\n"
        "}\n"
        "\n"
        "instance addition for vector {\n"
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
        "    c:vector = a.not_existing_function(b)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Function \"not_existing_function()\" was not defined in typeclass "
            "\"addition\" instantiation for type \"vector\".",
            20, 17, 20, 40)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typeclass_errors3) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "fn some_function() {\n"
        "    a:vector = vector(1, 2, 3)\n"
        "    b:vector = vector(3, 2, 1)\n"
        "    c:vector = a.not_existing_function(b)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "No typeclass instantiation for left type \"vector\" found. "
            "Can not call a method on a type without it.",
            6, 17, 6, 40)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typeclass_errors4) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "class addition <Type T> {\n"
        "    fn add(a:T, other:T) -> T\n"
        "}\n"
        "\n"
        "instance addition for vector {\n"
        "    fn add(a:vector, other:vector) -> vector\n"
        "    {\n"
        "        ret:vector\n"
        "        ret.x = a.x + other.x\n"
        "        ret.y = a.y + other.y\n"
        "        ret.z = a.z + other.z\n"
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

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Typeclass instantiation for function call \"add\" does not have "
            "'self' as the first argument.",
            4, 4, 4, 28)
    };

    ck_assert_typecheck_with_messages(false, messages);
} END_TEST

START_TEST(test_typeclass_errors5) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type vector {\n"
        "    x:int, y:int, z:int\n"
        "}\n"
        "fn add(self:vector, other:vector) -> vector\n"
        "{\n"
        "    ret:vector\n"
        "    ret.x = self.x + other.x\n"
        "    ret.y = self.y + other.y\n"
        "    ret.z = self.z + other.z\n"
        "    return ret\n"
        "}\n"
        "fn some_function() {\n"
        "    a:vector = vector(1, 2, 3)\n"
        "    b:vector = vector(3, 2, 1)\n"
        "    c:vector = a.add(b)\n"
        "}"
    );
    front_testdriver_new_ast_main_source(&s);

    struct info_msg messages[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH(
            MESSAGE_SEMANTIC_ERROR,
            "Reserved identifier 'self' used outside of a typeclass.",
            3, 7, 3, 10
        )
    };

    ck_assert_typecheck_with_messages(false, messages);
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
    tcase_add_test(t_typeclasses_simple, test_typeclass_2_instances_diff_types);
    tcase_add_test(t_typeclasses_simple, test_typeclass_2_instances_same_type);

    TCase *t_typeclasses_errors = tcase_create("typecheck_typeclasses_errors");
    tcase_add_checked_fixture(
        t_typeclasses_errors,
        setup_analyzer_tests,
        teardown_analyzer_tests
    );
    tcase_add_test(t_typeclasses_errors, test_typeclass_errors1);
    tcase_add_test(t_typeclasses_errors, test_typeclass_errors2);
    tcase_add_test(t_typeclasses_errors, test_typeclass_errors3);
    tcase_add_test(t_typeclasses_errors, test_typeclass_errors4);
    tcase_add_test(t_typeclasses_errors, test_typeclass_errors5);


    suite_add_tcase(s, t_typeclasses_simple);
    suite_add_tcase(s, t_typeclasses_errors);

    return s;
}
