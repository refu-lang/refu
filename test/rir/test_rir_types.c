#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ir/rir.h>
#include <ir/rir_types_list.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS



START_TEST (test_types_list_simple1) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn foo(a:u32) -> u64 {\n"
        "return 45 + a\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    struct rir_type *fn = testsupport_rir_type_create(COMPOSITE_IMPLICATION_RIR_TYPE, NULL, false);
    struct rir_type *fn_arg = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_32, &id_a, true);
    testsupport_rir_type_add_subtype(fn, fn_arg, false);
    struct rir_type *fn_ret = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, NULL, true);
    testsupport_rir_type_add_subtype(fn, fn_ret, true);

    struct rir_type *expected_types[] = {fn, fn_arg, fn_ret};
    rir_testdriver_compare_lists(expected_types);
} END_TEST

#if 0 // commenting this test out. TODO: Think if all subtypes
      // should also be included in rir types list too and if yes this test needs all products added
START_TEST(test_types_list_simple2) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:u32, b:u16, c:i8, d:f32, e:string}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(d, &s);
    ck_assert_typecheck_ok(d, false);

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
#endif

START_TEST(test_types_list_type_reuse) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}"
        "fn do_something(a:i64, b:f64) -> u32\n"
        "{\n"
        "return 15"
        "}\n"
        "type boo {a:i64, b:f64}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, true);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct rir_type *t_foo = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foo, false);
    testsupport_rir_type_add_subtype(t_foo, t_prod_1, true);

    struct rir_type *t_do_something = testsupport_rir_type_create(COMPOSITE_IMPLICATION_RIR_TYPE, NULL, false);
    struct rir_type *t_u32 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_32, NULL, true);
    testsupport_rir_type_add_subtype(t_do_something, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_do_something, t_u32, true);

    static const struct RFstring id_boo = RF_STRING_STATIC_INIT("boo");
    struct rir_type *t_boo = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_boo, false);
    testsupport_rir_type_add_subtype(t_boo, t_prod_1, true);

    struct rir_type *expected_types[] = {t_a_i64, t_b_f64, t_prod_1,
                                         t_do_something, t_foo, t_boo};
    rir_testdriver_compare_lists(expected_types);
} END_TEST

START_TEST(test_types_list_type_reuse_products_and_sums) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}\n"
        "type bar {a:i8, b:string}\n"
        "type foobar {a:i64, b:f64 | c:i8, d:string}\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, true);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct rir_type *t_foo = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foo, false);
    testsupport_rir_type_add_subtype(t_foo, t_prod_1, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_a_i8, false);
    struct rir_type *t_b_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_b_string, true);

    static const struct RFstring id_bar = RF_STRING_STATIC_INIT("bar");
    struct rir_type *t_bar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_bar, false);
    testsupport_rir_type_add_subtype(t_bar, t_prod_2, true);

    struct rir_type *t_prod_3 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_3, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_3, t_d_string, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_3, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_sum_1, true);

    struct rir_type *expected_types[] = {t_a_i64, t_b_f64, t_a_i8, t_b_string,
                                         t_c_i8, t_d_string, t_prod_1, t_prod_2,
                                         t_sum_1, t_foo, t_bar, t_foobar, t_prod_3
    };
    rir_testdriver_compare_lists(expected_types);
} END_TEST

START_TEST(test_rir_type_defined_always_one_subtype) {
    // This program was causing a bug where a composite rir type was found twice
    // in the rir types list and had 2 subtypes
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "module amod {\n"
        "type atype {\n"
        "a:u64, b:f32 | c:i64, d:u64\n"
        "}\n"
        "fn foo(a:atype)->u64 { return 1}\n"
        "}"
    );
    front_testdriver_new_source(&s);
    ck_assert_typecheck_ok();

    // make sure type is only once in the list and contains 1 subtype
    struct rir_type *t;
    bool found = false;
    rir_types_list_for_each(front_testdriver_rir()->rir_types_list, t) {
        if (t->category == COMPOSITE_RIR_DEFINED) {
            ck_assert_msg(!found, "atype was found twice in the rir types list");
            ck_assert_rf_str_eq_cstr(t->name, "atype");
            ck_assert_msg(darray_size(t->subtypes) == 1,
                          "Composite rir type \""RF_STR_PF_FMT"\" should only have 1 subtype",
                          RF_STR_PF_ARG(rir_type_str_or_die(t)));
            found = true;
        }
    }

} END_TEST

START_TEST(test_rir_type_equals_type1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foobar {a:i64, b:f64, c:i8, d:string}\n"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    // search the normal types for the only defined type that should be there
    
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(testsupport_rir_typeset(d), &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            break;
        }
    }
    ck_assert_rf_str_eq_cstr(t->defined.name, "foobar");

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_d_string, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_prod_1, true);

    // do the comparison
    ck_assert_rir_type_equals_type(t_foobar, t, NULL);
} END_TEST

START_TEST(test_rir_type_equals_type2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foobar {a:i64, b:f64 | c:i8, d:string}\n"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    // search the normal types for the only defined type that should be there
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(testsupport_rir_typeset(d), &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            break;
        }
    }
    ck_assert_rf_str_eq_cstr(t->defined.name, "foobar");

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_d_string, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_2, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_sum_1, true);

    // do the comparison
    ck_assert_rir_type_equals_type(t_foobar, t, NULL);
} END_TEST

START_TEST(test_rir_type_equals_type3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foobar {a:u64, b:f32 | c:i64, d:u64 | e:i8, f:u64}"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    // search the normal types for the only defined type that should be there
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(testsupport_rir_typeset(d), &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            break;
        }
    }
    ck_assert_rf_str_eq_cstr(t->defined.name, "foobar");

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    static const struct RFstring id_e = RF_STRING_STATIC_INIT("e");
    static const struct RFstring id_f = RF_STRING_STATIC_INIT("f");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_u64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_u64, false);
    struct rir_type *t_b_f32 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_32, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f32, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_c_i64, false);
    struct rir_type *t_d_u64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_d_u64, true);

    struct rir_type *t_prod_3 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_e_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_e, true);
    testsupport_rir_type_add_subtype(t_prod_3, t_e_i8, false);
    struct rir_type *t_f_u64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, &id_f, true);
    testsupport_rir_type_add_subtype(t_prod_3, t_f_u64, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_2, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_3, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_sum_1, true);

    // do the comparison
    ck_assert_rir_type_equals_type(t_foobar, t, NULL);
} END_TEST

START_TEST(test_rir_type_equals_type4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foobar {a:i64, b:f64, foo:i32 | c:i8, d:string | e:i8}\n"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    // search the normal types for the only defined type that should be there
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(testsupport_rir_typeset(d), &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            break;
        }
    }
    ck_assert_rf_str_eq_cstr(t->defined.name, "foobar");

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    static const struct RFstring id_e = RF_STRING_STATIC_INIT("e");
    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, false);
    struct rir_type *t_foo_i32 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_32, &id_foo, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_foo_i32, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_d_string, true);

    struct rir_type *t_e_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_e, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_2, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_e_i8, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_sum_1, true);

    // do the comparison
    ck_assert_rir_type_equals_type(t_foobar, t, NULL);
} END_TEST

START_TEST(test_rir_type_equals_type5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foobar {(a:i64, b:f64, foo:i32 | c:i8, d:string | e:i8) -> f:i32}\n"
    );
    struct rir_testdriver *d = get_rir_testdriver();
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    // search the normal types for the only defined type that should be there
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(testsupport_rir_typeset(d), &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            break;
        }
    }
    ck_assert_rf_str_eq_cstr(t->defined.name, "foobar");

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    static const struct RFstring id_e = RF_STRING_STATIC_INIT("e");
    static const struct RFstring id_f = RF_STRING_STATIC_INIT("f");
    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");

    struct rir_type *t_prod_1 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_a_i64, false);
    struct rir_type *t_b_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_b, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_b_f64, false);
    struct rir_type *t_foo_i32 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_32, &id_foo, true);
    testsupport_rir_type_add_subtype(t_prod_1, t_foo_i32, true);

    struct rir_type *t_prod_2 = testsupport_rir_type_create(COMPOSITE_PRODUCT_RIR_TYPE, NULL, false);
    struct rir_type *t_c_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_c, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_c_i8, false);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    testsupport_rir_type_add_subtype(t_prod_2, t_d_string, true);

    struct rir_type *t_e_i8 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_8, &id_e, true);

    struct rir_type *t_sum_1 = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_1, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_prod_2, false);
    testsupport_rir_type_add_subtype(t_sum_1, t_e_i8, true);

    struct rir_type *t_impl_1 = testsupport_rir_type_create(COMPOSITE_IMPLICATION_RIR_TYPE, NULL, false);
    struct rir_type *t_f_i32 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_32, &id_f, true);
    testsupport_rir_type_add_subtype(t_impl_1, t_sum_1, false);
    testsupport_rir_type_add_subtype(t_impl_1, t_f_i32, true);

    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    struct rir_type *t_foobar = testsupport_rir_type_create(COMPOSITE_RIR_DEFINED, &id_foobar, false);
    testsupport_rir_type_add_subtype(t_foobar, t_impl_1, true);

    // do the comparison
    ck_assert_rir_type_equals_type(t_foobar, t, NULL);
} END_TEST

START_TEST(test_rir_type_doesnotequal_subsum_type) {

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    // create normal type for (a:i64 | b:u64 | c:f64)
    struct type *n_a_i64 = testsupport_analyzer_type_create_leaf(
        &id_a,
        type_elementary_get_type(ELEMENTARY_TYPE_INT_64));
    struct type *n_b_u64 = testsupport_analyzer_type_create_leaf(
        &id_b,
        type_elementary_get_type(ELEMENTARY_TYPE_UINT_64));
    struct type *n_c_f64 = testsupport_analyzer_type_create_leaf(
        &id_c,
        type_elementary_get_type(ELEMENTARY_TYPE_FLOAT_64));
    struct type *n_sum1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   n_a_i64,
                                                                   n_b_u64);
    struct type *n_sum= testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                  n_sum1,
                                                                  n_c_f64);
    
    // create rir type for (a:i64 | b:u64 | c:f64 | d:string)
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    struct rir_type *t_b_u64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, &id_b, true);
    struct rir_type *t_c_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_c, true);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    struct rir_type *t_sum = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum, t_a_i64, false);
    testsupport_rir_type_add_subtype(t_sum, t_b_u64, false);
    testsupport_rir_type_add_subtype(t_sum, t_c_f64, false);
    testsupport_rir_type_add_subtype(t_sum, t_d_string, true);

    // the comparison should fail
    ck_assert(!rir_type_equals_type(t_sum, n_sum, NULL));
} END_TEST

START_TEST(test_rir_type_childof_type) {
    // create rir type for (a:i64 | b:u64 | c:f64 | d:string)
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    struct rir_type *t_a_i64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_INT_64, &id_a, true);
    struct rir_type *t_b_u64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_UINT_64, &id_b, true);
    struct rir_type *t_c_f64 = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_FLOAT_64, &id_c, true);
    struct rir_type *t_d_string = testsupport_rir_type_create(ELEMENTARY_RIR_TYPE_STRING, &id_d, true);
    struct rir_type *t_sum = testsupport_rir_type_create(COMPOSITE_SUM_RIR_TYPE, NULL, false);
    testsupport_rir_type_add_subtype(t_sum, t_a_i64, false);
    testsupport_rir_type_add_subtype(t_sum, t_b_u64, false);
    testsupport_rir_type_add_subtype(t_sum, t_c_f64, false);
    testsupport_rir_type_add_subtype(t_sum, t_d_string, true);

    // test that each subtype has the correct index with rir_type_childof_type
    ck_assert_int_eq(0, rir_type_childof_type(t_a_i64, t_sum));
    ck_assert_int_eq(1, rir_type_childof_type(t_b_u64, t_sum));
    ck_assert_int_eq(2, rir_type_childof_type(t_c_f64, t_sum));
    ck_assert_int_eq(3, rir_type_childof_type(t_d_string, t_sum));

} END_TEST

Suite *rir_types_suite_create(void)
{
    Suite *s = suite_create("rir_types");

    TCase *type_lists = tcase_create("rir_types_list_creation");
    tcase_add_checked_fixture(type_lists,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(type_lists, test_types_list_simple1);
    /* tcase_add_test(type_lists, test_types_list_simple2); */
    tcase_add_test(type_lists, test_types_list_type_reuse);
    tcase_add_test(type_lists, test_types_list_type_reuse_products_and_sums);
    tcase_add_test(type_lists, test_rir_type_defined_always_one_subtype);

    TCase *type_comparison = tcase_create("rir_types_comparison");
    tcase_add_checked_fixture(type_comparison,
                              setup_rir_tests_no_stdlib,
                              teardown_rir_tests);
    tcase_add_test(type_comparison, test_rir_type_equals_type1);
    tcase_add_test(type_comparison, test_rir_type_equals_type2);
    tcase_add_test(type_comparison, test_rir_type_equals_type3);
    tcase_add_test(type_comparison, test_rir_type_equals_type4);
    tcase_add_test(type_comparison, test_rir_type_equals_type5);

    TCase *type_misc = tcase_create("rir_types_misc_no_source");
    tcase_add_checked_fixture(type_misc,
                              setup_rir_tests_no_source,
                              teardown_rir_tests);
    tcase_add_test(type_misc, test_rir_type_childof_type);
    tcase_add_test(type_misc, test_rir_type_doesnotequal_subsum_type);


    suite_add_tcase(s, type_lists);
    suite_add_tcase(s, type_comparison);
    suite_add_tcase(s, type_misc);

    return s;
}
