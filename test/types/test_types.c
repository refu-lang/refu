#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <types/type.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "../analyzer/testsupport_analyzer.h"

#include CLIB_TEST_HELPERS


START_TEST (test_type_to_str) {

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_leaf_u32 = testsupport_analyzer_type_create_leaf(&id_foo, t_u32);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_boo, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_leaf_u32,
                                                                     t_leaf_f64);

    RFS_buffer_push();
    ck_assert_rf_str_eq_cstr(type_str(t_u32, true), "u32");
    ck_assert_rf_str_eq_cstr(type_str(t_leaf_u32, true), "foo:u32");
    ck_assert_rf_str_eq_cstr(type_str(t_prod_1, true), "foo:u32,boo:f64");
    ck_assert_rf_str_eq_cstr(type_str(t_prod_1, false), "u32,f64");
    RFS_buffer_pop();
} END_TEST

#if 0
START_TEST(test_composite_types_list_population) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}"
        "fn do_something() -> u32\n"
        "{\n"
        "return 15"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d);

    struct analyzer *a = d->front.analyzer;
    struct type *t;
    rf_ilist_for_each(&a->composite_types, t, lh) {
        int a = 5;
    }
} END_TEST
#endif

Suite *types_suite_create(void)
{
    Suite *s = suite_create("types");

    TCase *st1 = tcase_create("types_general_tests");
    tcase_add_checked_fixture(st1, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_to_str);

    TCase *st2 = tcase_create("types_management_tests");
    tcase_add_checked_fixture(st2, setup_analyzer_tests, teardown_analyzer_tests);
    #if 0
    tcase_add_test(st2, test_composite_types_list_population);
    #endif

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}
