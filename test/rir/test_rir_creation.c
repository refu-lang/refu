#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS



START_TEST(test_types_list_simple1) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:u32) -> u64 {\n"
        "return 45 + a\n"
        "}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    rir_testdriver_assign(d, &s);
    testsupport_rir_process(d);

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    struct rir_type *fn = testsupport_rir_type_create(d, COMPOSITE_IMPLICATION_RIR_TYPE, NULL, false);
    struct rir_type *fn_arg = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_UINT_32, &id_a, true);
    testsupport_rir_type_add_subtype(d, fn, fn_arg, false);
    struct rir_type *fn_ret = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_UINT_64, NULL, true);
    testsupport_rir_type_add_subtype(d, fn, fn_ret, true);

    struct rir_type *expected_types[] = {fn, fn_arg};
    rir_testdriver_compare_lists(d, expected_types);
} END_TEST

START_TEST(test_types_list_simple2) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:u32, b:u16, c:i8, d:f32, e:string}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    rir_testdriver_assign(d, &s);
    testsupport_rir_process(d);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    static const struct RFstring id_e = RF_STRING_STATIC_INIT("e");

    struct rir_type *t_foo = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_foo, false);
    struct rir_type *t_prod5 = testsupport_rir_type_create(d, COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_UINT_32, &id_a, true);
    testsupport_rir_type_add_subtype(d, t_prod5, t_a, false);
    struct rir_type *t_b = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_UINT_16, &id_b, true);
    testsupport_rir_type_add_subtype(d, t_prod5, t_b, false);
    struct rir_type *t_c = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(d, t_prod5, t_c, false);
    struct rir_type *t_d = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_FLOAT_32, &id_d, true);
    testsupport_rir_type_add_subtype(d, t_prod5, t_d, false);
    struct rir_type *t_e = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_STRING, &id_e, true);
    testsupport_rir_type_add_subtype(d, t_prod5, t_e, true);
    testsupport_rir_type_add_subtype(d, t_foo, t_prod5, true);

    struct rir_type *expected_types[] = {t_a, t_b, t_c, t_d, t_e, t_foo, t_prod5};
    rir_testdriver_compare_lists(d, expected_types);
} END_TEST

START_TEST(test_types_list_type_reuse) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}"
        "fn do_something(a:i64, b:f64) -> u32\n"
        "{\n"
        "return 15"
        "}\n"
        "type boo {a:i64, b:f64}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    rir_testdriver_assign(d, &s);
    testsupport_rir_process(d);

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    struct rir_type *t_prod_1 = testsupport_rir_type_create(d, COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(d, t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(d, t_prod_1, t_b_f64, true);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct rir_type *t_foo = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_foo, false);
    testsupport_rir_type_add_subtype(d, t_foo, t_prod_1, true);

    struct rir_type *t_do_something = testsupport_rir_type_create(d, COMPOSITE_IMPLICATION_RIR_TYPE, NULL, false);
    struct rir_type *t_u32 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_UINT_32, NULL, true);
    testsupport_rir_type_add_subtype(d, t_do_something, t_prod_1, false);
    testsupport_rir_type_add_subtype(d, t_do_something, t_u32, true);

    static const struct RFstring id_boo = RF_STRING_STATIC_INIT("boo");
    struct rir_type *t_boo = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_boo, false);
    testsupport_rir_type_add_subtype(d, t_boo, t_prod_1, true);

    struct rir_type *expected_types[] = {t_a_i64, t_b_f64, t_prod_1,
                                         t_do_something, t_foo, t_boo};
    rir_testdriver_compare_lists(d, expected_types);
} END_TEST

START_TEST(test_types_list_type_reuse_products_and_sums) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}\n"
        "type bar {a:i8, b:string}\n"
        "type foobar {a:i64, b:f64 | c:i8, d:string}\n"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    rir_testdriver_assign(d, &s);
    testsupport_rir_process(d);

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(d, COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(d, t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(d, t_prod_1, t_b_f64, true);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct rir_type *t_foo = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_foo, false);
    testsupport_rir_type_add_subtype(d, t_foo, t_prod_1, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(d, COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i8 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_INT_8, &id_a, true);
    testsupport_rir_type_add_subtype(d, t_prod_2, t_a_i8, false);
    struct rir_type *t_b_string = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_STRING, &id_b, true);
    testsupport_rir_type_add_subtype(d, t_prod_2, t_b_string, true);

    static const struct RFstring id_bar = RF_STRING_STATIC_INIT("bar");
    struct rir_type *t_bar = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_bar, false);
    testsupport_rir_type_add_subtype(d, t_bar, t_prod_2, true);

    struct rir_type *t_prod_3 = testsupport_rir_type_create(d, COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(d, t_prod_3, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(d, ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(d, t_prod_3, t_d_string, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(d, COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(d, t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(d, t_sum_1, t_prod_3, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(d, COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(d, t_foobar, t_sum_1, true);

    struct rir_type *expected_types[] = {t_a_i64, t_b_f64, t_a_i8, t_b_string,
                                         t_c_i8, t_d_string, t_prod_1, t_prod_2,
                                         t_sum_1, t_foo, t_bar, t_foobar, t_prod_3
    };
    rir_testdriver_compare_lists(d, expected_types);
} END_TEST


Suite *rir_creation_suite_create(void)
{
    Suite *s = suite_create("rir_creation");

    TCase *st_basic = tcase_create("rir_creation_types_list");
    tcase_add_checked_fixture(st_basic,
                              setup_rir_tests,
                              teardown_rir_tests);
    tcase_add_test(st_basic, test_types_list_simple1);
    tcase_add_test(st_basic, test_types_list_simple2);
    tcase_add_test(st_basic, test_types_list_type_reuse);
    tcase_add_test(st_basic, test_types_list_type_reuse_products_and_sums);

    suite_add_tcase(s, st_basic);

    return s;
}
