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

void ck_assert_type_lists_equal_impl(const struct type **expected_types,
                                     size_t expected_types_size,
                                     struct analyzer *a,
                                     const char *filename,
                                     unsigned int line)
{
    struct type *t;
    unsigned int i;
    struct type_comparison_ctx cmp_ctx;
    bool found;
    unsigned int found_types_size = 0;
    bool *found_indexes;
    RF_CALLOC(found_indexes, expected_types_size, sizeof(*found_indexes), return);

    RFS_buffer_push();
    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_IDENTICAL);
    rf_ilist_for_each(&a->composite_types, t, lh) {
        found = false;
        for (i = 0; i < expected_types_size; ++i) {
            if (type_equals(t, expected_types[i], &cmp_ctx)) {
                found = true;
                ck_assert_msg(!found_indexes[i],
                              "Found a duplicate type entry in the list. Type: "
                              RF_STR_PF_FMT,
                              RF_STR_PF_ARG(type_str(expected_types[i], true)));
                found_indexes[i] = true;
                break;
            }
        }
        if (!found) {
            ck_abort_msg("Did not manage to find type "RF_STR_PF_FMT" "
                         "in the expected types list from %s:%u",
                         RF_STR_PF_ARG(type_str(t, true)), filename, line);
        }
        found_types_size ++;
    }

    for (i = 0; i < expected_types_size; ++i) {
            ck_assert_msg(found_indexes[i],
                          "Expected type "RF_STR_PF_FMT" was not found in the "
                          "composite types list from %s:%u",
                          RF_STR_PF_ARG(type_str(expected_types[i], true)),
                          filename, line);
    }

    RFS_buffer_pop();
    free(found_indexes);
}

#define ck_assert_type_lists_equal(i_expected_types_, i_analyzer_)      \
    ck_assert_type_lists_equal_impl(i_expected_types_,                  \
                                    sizeof(i_expected_types_) / sizeof(struct type*), \
                                    i_analyzer_, __FILE__, __LINE__)


START_TEST (test_type_to_str) {

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_leaf_u32 = testsupport_analyzer_type_create_leaf(&id_foo, t_u32);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_boo, t_f64);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING);
    static const struct RFstring id_goo =  RF_STRING_STATIC_INIT("goo");
    struct type *t_leaf_string = testsupport_analyzer_type_create_leaf(&id_goo, t_string);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_leaf_u32,
                                                                     t_leaf_f64);

    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                     t_leaf_f64,
                                                                     t_leaf_string);

    static const struct RFstring id_person =  RF_STRING_STATIC_INIT("person");
    struct type *t_defined_1 = testsupport_analyzer_type_create_defined(&id_person,
                                                                        t_sum_1);

    RFS_buffer_push();
    ck_assert_rf_str_eq_cstr(type_str(t_u32, true), "u32");
    ck_assert_rf_str_eq_cstr(type_str(t_leaf_u32, true), "foo:u32");
    ck_assert_rf_str_eq_cstr(type_str(t_prod_1, true), "foo:u32,boo:f64");
    ck_assert_rf_str_eq_cstr(type_str(t_prod_1, false), "u32,f64");
    ck_assert_rf_str_eq_cstr(type_str(t_defined_1, false), "person");
    RFS_buffer_pop();
} END_TEST

START_TEST (test_type_comparison_identical) {

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64);
    struct type *t_leaf_ai8 = testsupport_analyzer_type_create_leaf(&id_a, t_i8);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);
    struct type *t_leaf_ci8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING);
    struct type *t_leaf_bstring = testsupport_analyzer_type_create_leaf(&id_b, t_string);
    struct type *t_leaf_dstring = testsupport_analyzer_type_create_leaf(&id_d, t_string);

    struct type_comparison_ctx cmp_ctx;
    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_IDENTICAL);

    // normal type comparison should work
    ck_assert(type_equals(t_leaf_ai8, t_leaf_ci8, NULL));
    ck_assert(type_equals(t_leaf_bstring, t_leaf_dstring, NULL));

    // detect identical types
    ck_assert(type_equals(t_leaf_ai8, t_leaf_ai8, &cmp_ctx));
    ck_assert(type_equals(t_leaf_dstring, t_leaf_dstring, &cmp_ctx));

    // detect equal but not identical types
    ck_assert(!type_equals(t_leaf_ai8, t_leaf_ci8, &cmp_ctx));
    ck_assert(!type_equals(t_leaf_bstring, t_leaf_dstring, &cmp_ctx));

    // make sure that if only name is same we consider it neither equal nor identical
    ck_assert(!type_equals(t_leaf_ai8, t_leaf_ai64, NULL));
    ck_assert(!type_equals(t_leaf_ai8, t_leaf_ai64, &cmp_ctx));

} END_TEST

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
    ck_assert_typecheck_ok(d, false);

    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_a =  RF_STRING_STATIC_INIT("a");
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    static const struct RFstring id_b =  RF_STRING_STATIC_INIT("b");
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_i64,
                                                                      t_leaf_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_nil,
                                                                      t_u32);

    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_leaf_i64, t_leaf_f64,
                                             t_func_1, t_foo };
    ck_assert_type_lists_equal(expected_types, d->front.analyzer);
} END_TEST

START_TEST(test_composite_types_list_population2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}"
        "fn do_something(a:i64, b:f64) -> u32\n"
        "{\n"
        "return 15"
        "}\n"
        "type boo {a:i64, b:f64}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, false);

    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    static const struct RFstring id_a =  RF_STRING_STATIC_INIT("a");

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    static const struct RFstring id_b =  RF_STRING_STATIC_INIT("b");
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_i64,
                                                                      t_leaf_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_prod_1,
                                                                      t_u32);

    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);
    struct type *t_boo = testsupport_analyzer_type_create_defined(&id_boo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_leaf_i64, t_leaf_f64,
                                             t_func_1, t_foo, t_boo};
    ck_assert_type_lists_equal(expected_types, d->front.analyzer);
} END_TEST

START_TEST(test_composite_types_list_population3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64, c:i8, d:f32, e:string}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, false);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");
    static const struct RFstring id_e = RF_STRING_STATIC_INIT("e");

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8);
    struct type *t_leaf_i8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32);
    struct type *t_leaf_f32 = testsupport_analyzer_type_create_leaf(&id_d, t_f32);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING);
    struct type *t_leaf_string = testsupport_analyzer_type_create_leaf(&id_e, t_string);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_i64,
                                                                      t_leaf_f64);
    struct type *t_prod_2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_prod_1,
                                                                      t_leaf_i8);
    struct type *t_prod_3 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_prod_2,
                                                                      t_leaf_f32);
    struct type *t_prod_4 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_prod_3,
                                                                      t_leaf_string);
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_4);

    const struct type *expected_types [] = { t_leaf_i64, t_leaf_f64, t_leaf_i8,
                                             t_leaf_f32, t_leaf_string, t_prod_4,
                                             t_foo};
    ck_assert_type_lists_equal(expected_types, d->front.analyzer);
} END_TEST

START_TEST(test_composite_types_list_population4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}\n"
        "type bar {a:i8, b:string}\n"
        "type foobar {a:i64, b:f64 | c:i8, d:string}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, false);

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_bar = RF_STRING_STATIC_INIT("bar");
    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64);
    struct type *t_leaf_bf64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8);
    struct type *t_leaf_ai8 = testsupport_analyzer_type_create_leaf(&id_a, t_i8);
    struct type *t_leaf_ci8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING);
    struct type *t_leaf_bstring = testsupport_analyzer_type_create_leaf(&id_b, t_string);
    struct type *t_leaf_dstring = testsupport_analyzer_type_create_leaf(&id_d, t_string);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_ai64,
                                                                      t_leaf_bf64);
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    struct type *t_prod_2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_ai8,
                                                                      t_leaf_bstring);
    struct type *t_bar = testsupport_analyzer_type_create_defined(&id_bar, t_prod_2);

    struct type *t_prod_3 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_ci8,
                                                                      t_leaf_dstring);
    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                      t_prod_1,
                                                                      t_prod_3);
    struct type *t_foobar = testsupport_analyzer_type_create_defined(&id_foobar, t_sum_1);

    const struct type *expected_types [] = { t_leaf_ai64, t_leaf_bf64, t_leaf_ai8,
                                             t_leaf_bstring, t_leaf_ci8, t_leaf_dstring,
                                             t_prod_1, t_prod_2, t_sum_1,
                                             t_foo, t_bar, t_foobar
                                             /* t_prod_3*/ // TODO: We need this to be there
    };
    ck_assert_type_lists_equal(expected_types, d->front.analyzer);
} END_TEST


Suite *types_suite_create(void)
{
    Suite *s = suite_create("types");

    TCase *st1 = tcase_create("types_general_tests");
    tcase_add_checked_fixture(st1, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_to_str);
    tcase_add_test(st1, test_type_comparison_identical);

    TCase *st2 = tcase_create("types_management_tests");
    tcase_add_checked_fixture(st2, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st2, test_composite_types_list_population);
    tcase_add_test(st2, test_composite_types_list_population2);
    tcase_add_test(st2, test_composite_types_list_population3);
    tcase_add_test(st2, test_composite_types_list_population4);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}
