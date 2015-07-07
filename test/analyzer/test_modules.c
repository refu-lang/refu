#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <info/msg.h>

#include <ast/function.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
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

START_TEST (test_multiple_dependencies) {
    static const struct RFstring base = RF_STRING_STATIC_INIT(
        "module base {}"
    );
    static const struct RFstring pack1 = RF_STRING_STATIC_INIT(
        "module package1 { import base }\n"
    );
    static const struct RFstring pack2 = RF_STRING_STATIC_INIT(
        "module package2 { import base }\n"
    );
    static const struct RFstring child = RF_STRING_STATIC_INIT(
        "module child {\n"
        "    import package1\n"
        "    import package2\n"
        "}\n"
    );
    front_testdriver_new_source(&child);
    front_testdriver_new_source(&pack1);
    front_testdriver_new_source(&base);
    front_testdriver_new_source(&pack2);

    static const struct RFstring expected_modules[] = {
        RF_STRING_STATIC_INIT("base"),
        RF_STRING_STATIC_INIT("package2"),
        RF_STRING_STATIC_INIT("package1"),
        RF_STRING_STATIC_INIT("child"),
    };
    ck_assert_modules_order(expected_modules);

} END_TEST

START_TEST (test_complicated_dependencies) {
    static const struct RFstring b = RF_STRING_STATIC_INIT(
        "module b {\n"
        "    import f\n"
        "    import a\n"
        "    import e\n"
        "    import c\n"
        "}"
    );
    static const struct RFstring f = RF_STRING_STATIC_INIT(
        "module f {\n"
        "    import a\n"
        "}"
    );
    static const struct RFstring a = RF_STRING_STATIC_INIT(
        "module a {\n"
        "}"
    );
    static const struct RFstring c = RF_STRING_STATIC_INIT(
        "module c {\n"
        "    import g\n"
        "}"
    );
    static const struct RFstring g = RF_STRING_STATIC_INIT(
        "module g {\n"
        "}"
    );
    static const struct RFstring d = RF_STRING_STATIC_INIT(
        "module d {\n"
        "    import f\n"
        "    import b\n"
        "    import e\n"
        "    import g\n"
        "}"
    );
    static const struct RFstring e = RF_STRING_STATIC_INIT(
        "module e {\n"
        "}"
    );

    front_testdriver_new_source(&a);
    front_testdriver_new_source(&b);
    front_testdriver_new_source(&c);
    front_testdriver_new_source(&d);
    front_testdriver_new_source(&e);
    front_testdriver_new_source(&f);
    front_testdriver_new_source(&g);

    static const struct RFstring expected_modules[] = {
        RF_STRING_STATIC_INIT("g"),
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("f"),
        RF_STRING_STATIC_INIT("e"),
        RF_STRING_STATIC_INIT("c"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("d"),
    };
    ck_assert_modules_order(expected_modules);

} END_TEST

START_TEST (test_simple_cycle) {
    static const struct RFstring a = RF_STRING_STATIC_INIT(
        "module a { \n"
        "    import b\n"
        "}\n"
    );
    static const struct RFstring b = RF_STRING_STATIC_INIT(
        "module b {\n"
        "    import a\n"
        "}\n"
    );
    front_testdriver_new_source(&a);
    front_testdriver_new_source(&b);

    ck_assert_modules_cyclic_dependency_detected(
        0, // front_ctx/file index
        "Cyclic dependency around module \"b\" detected.", 0, 0, 2, 0
    );

} END_TEST

START_TEST (test_cycle_in_big_graph) {
    static const struct RFstring b = RF_STRING_STATIC_INIT(
        "module b {\n"
        "    import f\n"
        "    import a\n"
        "    import e\n"
        "    import c\n"
        "}"
    );
    static const struct RFstring f = RF_STRING_STATIC_INIT(
        "module f {\n"
        "    import a\n"
        "}"
    );
    static const struct RFstring a = RF_STRING_STATIC_INIT(
        "module a {\n"
        "    import d\n"
        "}"
    );
    static const struct RFstring c = RF_STRING_STATIC_INIT(
        "module c {\n"
        "    import g\n"
        "}"
    );
    static const struct RFstring g = RF_STRING_STATIC_INIT(
        "module g {\n"
        "}"
    );
    static const struct RFstring d = RF_STRING_STATIC_INIT(
        "module d {\n"
        "    import f\n"
        "    import b\n"
        "    import e\n"
        "    import g\n"
        "}"
    );
    static const struct RFstring e = RF_STRING_STATIC_INIT(
        "module e {\n"
        "}"
    );

    front_testdriver_new_source(&a);
    front_testdriver_new_source(&b);
    front_testdriver_new_source(&c);
    front_testdriver_new_source(&d);
    front_testdriver_new_source(&e);
    front_testdriver_new_source(&f);
    front_testdriver_new_source(&g);

    ck_assert_modules_cyclic_dependency_detected(
        1, // front_ctx/file index
        "Cyclic dependency around module \"f\" detected.", 0, 0, 2, 0
    );

} END_TEST

START_TEST (test_modules_same_name) {
    static const struct RFstring foo = RF_STRING_STATIC_INIT(
        "module foo {\n"
        "}"
    );
    static const struct RFstring foo2 = RF_STRING_STATIC_INIT(
        "module foo {\n"
        "}"
    );

    front_testdriver_new_source(&foo);
    front_testdriver_new_source(&foo2);

    ck_assert_modules_cyclic_dependency_detected(
        1, // front_ctx/file index
        "Module \"foo\" already declared", 0, 0, 1, 0
    );

} END_TEST

START_TEST (test_modules_nonexistent_import) {
    static const struct RFstring foo = RF_STRING_STATIC_INIT(
        "module foo {\n"
        "}"
    );
    static const struct RFstring boo = RF_STRING_STATIC_INIT(
        "module boo {\n"
        "    import foo\n"
        "    import nonexistant\n"
        "}"
    );

    front_testdriver_new_source(&foo);
    front_testdriver_new_source(&boo);

    ck_assert_modules_cyclic_dependency_detected(
        0, // front_ctx/file index
        "Requested module \"nonexistant\" not found for importing.", 2, 4, 2, 21
    );

} END_TEST

START_TEST (test_modules_main_detection) {
    static const struct RFstring mainm = RF_STRING_STATIC_INIT(
        "fn main() -> u32 { }\n"
        "fn other() -> string {}"
    );
    static const struct RFstring boo = RF_STRING_STATIC_INIT(
        "module boo {\n"
        "}"
    );

    struct front_ctx *mfront = front_testdriver_new_source(&mainm);
    front_testdriver_new_source(&boo);
    ck_assert_typecheck_ok();

    ck_assert_msg(mfront->is_main, "Main module not detected succesfully");

} END_TEST

START_TEST (test_modules_multiple_main_error) {
    static const struct RFstring mainm = RF_STRING_STATIC_INIT(
        "fn main() -> u32 { }\n"
        "fn other() -> string {}"
    );
    static const struct RFstring boo = RF_STRING_STATIC_INIT(
        "module boo {\n"
        "fn main() -> i32 { }\n"
        "}"
    );

    front_testdriver_new_source(&mainm);
    front_testdriver_new_source(&boo);

    struct info_msg errors[] = {
        TESTSUPPORT_INFOMSG_INIT_BOTH_SPECIFIC_FRONT(
            1,
            MESSAGE_SEMANTIC_ERROR,
            "Multiple definition of main() detected in \"test_filename\". Previous "
            "definition was in \"test_filename\".",
            0, 0, 0, 0)
    };
    ck_test_parse_fronts(false, errors);
} END_TEST

Suite *analyzer_modules_suite_create(void)
{
    Suite *s = suite_create("analyzer_modules");

    TCase *t_1 = tcase_create("modules_dependency_order");
    tcase_add_checked_fixture(t_1,
                              setup_analyzer_tests_no_stdlib,
                              teardown_analyzer_tests);
    tcase_add_test(t_1, test_single_dependency);
    tcase_add_test(t_1, test_multiple_dependencies);
    tcase_add_test(t_1, test_complicated_dependencies);

    TCase *t_2 = tcase_create("modules_dependency_cycles");
    tcase_add_checked_fixture(t_2,
                              setup_analyzer_tests_no_stdlib,
                              teardown_analyzer_tests);
    tcase_add_test(t_2, test_simple_cycle);
    tcase_add_test(t_2, test_cycle_in_big_graph);


    TCase *t_3 = tcase_create("modules_other_errors");
    tcase_add_checked_fixture(t_3,
                              setup_analyzer_tests_no_stdlib,
                              teardown_analyzer_tests);
    tcase_add_test(t_3, test_modules_same_name);
    tcase_add_test(t_3, test_modules_nonexistent_import);

    TCase *t_4 = tcase_create("modules_analysis_misc");
    tcase_add_checked_fixture(t_4,
                              setup_analyzer_tests_no_stdlib,
                              teardown_analyzer_tests);
    tcase_add_test(t_4, test_modules_main_detection);
    tcase_add_test(t_4, test_modules_multiple_main_error);

    suite_add_tcase(s, t_1);
    suite_add_tcase(s, t_2);
    suite_add_tcase(s, t_3);
    suite_add_tcase(s, t_4);
    return s;
}
