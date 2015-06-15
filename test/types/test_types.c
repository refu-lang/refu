#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <types/type.h>
#include <types/type_elementary.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "../analyzer/testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

static void ck_assert_type_set_equal_impl(const struct type **expected_types,
                                          size_t expected_types_size,
                                          struct analyzer *a,
                                          const char *filename,
                                          unsigned int line)
{
    struct type *t;
    struct rf_objset_iter it;
    unsigned int i;
    bool found;
    unsigned int found_types_size = 0;
    bool *found_indexes;
    RF_CALLOC(found_indexes, expected_types_size, sizeof(*found_indexes), return);

    rf_objset_foreach(a->types_set, &it, t) {
        found = false;
        for (i = 0; i < expected_types_size; ++i) {
            if (type_compare(t, expected_types[i], TYPECMP_IDENTICAL)) {
                found = true;

                RFS_PUSH();
                ck_assert_msg(!found_indexes[i],
                              "Found a duplicate type entry in the list. Type: "
                              RF_STR_PF_FMT,
                              RF_STR_PF_ARG(type_str_or_die(expected_types[i], TSTR_LEAF_ID)));
                found_indexes[i] = true;
                RFS_POP();
                break;
            }
        }
        if (!found) {
            ck_abort_msg("Did not manage to find type "RF_STR_PF_FMT" "
                         "in the expected types list from %s:%u",
                         RF_STR_PF_ARG(type_str_or_die(t, TSTR_LEAF_ID)),
                         filename,
                         line);
        }
        found_types_size ++;
    }

    for (i = 0; i < expected_types_size; ++i) {
        RFS_PUSH();
        ck_assert_msg(found_indexes[i],
                      "Expected type "RF_STR_PF_FMT" was not found in the "
                      "composite types list from %s:%u",
                      RF_STR_PF_ARG(type_str_or_die(expected_types[i], TSTR_LEAF_ID)),
                      filename, line);
        RFS_POP();
    }
    free(found_indexes);
}

#define ck_assert_type_set_equal(i_expected_types_, i_analyzer_)      \
    ck_assert_type_set_equal_impl(i_expected_types_,                  \
                                  sizeof(i_expected_types_) / sizeof(struct type*), \
                                  i_analyzer_, __FILE__, __LINE__)


START_TEST (test_type_to_str) {

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_leaf_u32 = testsupport_analyzer_type_create_leaf(&id_foo, t_u32);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_boo, t_f64);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
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

    struct RFstring *ts;
    RFS_PUSH();
    ck_assert((ts = type_str(t_u32, TSTR_LEAF_ID)));
    ck_assert_rf_str_eq_cstr(ts, "u32");
    ck_assert((ts = type_str(t_u32, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "u32");
    ck_assert((ts = type_str(t_leaf_u32, TSTR_LEAF_ID)));
    ck_assert_rf_str_eq_cstr(ts, "foo:u32");
    ck_assert((ts = type_str(t_prod_1, TSTR_LEAF_ID)));
    ck_assert_rf_str_eq_cstr(ts, "foo:u32,boo:f64");
    ck_assert((ts = type_str(t_prod_1, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "u32,f64");
    ck_assert((ts = type_str(t_defined_1, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "person");
    RFS_POP();
} END_TEST

START_TEST (test_type_comparison_identical) {

    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_ai8 = testsupport_analyzer_type_create_leaf(&id_a, t_i8);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);
    struct type *t_leaf_ci8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_leaf_bstring = testsupport_analyzer_type_create_leaf(&id_b, t_string);
    struct type *t_leaf_dstring = testsupport_analyzer_type_create_leaf(&id_d, t_string);


    // normal type comparison should work
    ck_assert(type_compare(t_leaf_ai8, t_leaf_ci8, TYPECMP_IMPLICIT_CONVERSION));
    ck_assert(type_compare(t_leaf_bstring, t_leaf_dstring, TYPECMP_IMPLICIT_CONVERSION));

    // detect identical types
    ck_assert(type_compare(t_leaf_ai8, t_leaf_ai8, TYPECMP_IDENTICAL));
    ck_assert(type_compare(t_leaf_dstring, t_leaf_dstring, TYPECMP_IDENTICAL));

    // detect equal but not identical types
    ck_assert(!type_compare(t_leaf_ai8, t_leaf_ci8, TYPECMP_IDENTICAL));
    ck_assert(!type_compare(t_leaf_bstring, t_leaf_dstring, TYPECMP_IDENTICAL));

    // make sure that if only name is same we don't consider it equal
    ck_assert(!type_compare(t_leaf_ai8, t_leaf_ai64, TYPECMP_IDENTICAL));

} END_TEST

START_TEST (test_type_comparison_for_sum_fncall) {
    // test for a bug concerning subtypes of sum types
    // make type a:i64 | b:u64 | c:f64 | d:string
    // just like the typechecking for function calls
    // check that comparing a subtype gives the correct matched type
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_leaf_bu64 = testsupport_analyzer_type_create_leaf(&id_b, t_u64);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_cf64 = testsupport_analyzer_type_create_leaf(&id_c, t_f64);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_leaf_dstring = testsupport_analyzer_type_create_leaf(&id_d, t_string);

    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                     t_leaf_ai64,
                                                                     t_leaf_bu64);
    struct type *t_sum_2 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                     t_sum_1,
                                                                     t_leaf_cf64);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_sum_2,
                                                                   t_leaf_dstring);

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_leaf_bu64, t_sum, TYPECMP_PATTERN_MATCHING));
    const struct type *matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_leaf_bu64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_leaf_ai64, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_leaf_ai64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_leaf_cf64, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_leaf_cf64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_leaf_dstring, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_leaf_dstring, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

} END_TEST

START_TEST (test_type_comparison_for_sum_fncall_with_conversion) {
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id_c = RF_STRING_STATIC_INIT("c");
    static const struct RFstring id_d = RF_STRING_STATIC_INIT("d");

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_cf64 = testsupport_analyzer_type_create_leaf(&id_c, t_f64);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_leaf_dstring = testsupport_analyzer_type_create_leaf(&id_d, t_string);

    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                     t_leaf_ai64,
                                                                     t_leaf_cf64);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_sum_1,
                                                                   t_leaf_dstring);

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_i8, t_sum, TYPECMP_PATTERN_MATCHING));
    const struct type *matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_i64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));
} END_TEST

START_TEST (test_elementary_get_category) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert_int_eq(type_elementary_get_category(t_i), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i8), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u8), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i16), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u16), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i32), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u32), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i64), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u64), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_f32), ELEMENTARY_TYPE_CATEGORY_FLOAT);
    ck_assert_int_eq(type_elementary_get_category(t_f64), ELEMENTARY_TYPE_CATEGORY_FLOAT);
    ck_assert_int_eq(type_elementary_get_category(t_string), ELEMENTARY_TYPE_CATEGORY_OTHER);
    ck_assert_int_eq(type_elementary_get_category(t_bool), ELEMENTARY_TYPE_CATEGORY_OTHER);
    ck_assert_int_eq(type_elementary_get_category(t_nil), ELEMENTARY_TYPE_CATEGORY_OTHER);
} END_TEST

START_TEST (test_is_signed_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(type_is_signed_elementary(t_i));
    ck_assert(!type_is_signed_elementary(t_u));
    ck_assert(type_is_signed_elementary(t_i8));
    ck_assert(!type_is_signed_elementary(t_u8));
    ck_assert(type_is_signed_elementary(t_i16));
    ck_assert(!type_is_signed_elementary(t_u16));
    ck_assert(type_is_signed_elementary(t_i32));
    ck_assert(!type_is_signed_elementary(t_u32));
    ck_assert(type_is_signed_elementary(t_i64));
    ck_assert(!type_is_signed_elementary(t_u64));
    ck_assert(!type_is_signed_elementary(t_f32));
    ck_assert(!type_is_signed_elementary(t_f64));
    ck_assert(!type_is_signed_elementary(t_string));
    ck_assert(!type_is_signed_elementary(t_bool));
    ck_assert(!type_is_signed_elementary(t_nil));
} END_TEST

START_TEST (test_is_unsigned_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(!type_is_unsigned_elementary(t_i));
    ck_assert(type_is_unsigned_elementary(t_u));
    ck_assert(!type_is_unsigned_elementary(t_i8));
    ck_assert(type_is_unsigned_elementary(t_u8));
    ck_assert(!type_is_unsigned_elementary(t_i16));
    ck_assert(type_is_unsigned_elementary(t_u16));
    ck_assert(!type_is_unsigned_elementary(t_i32));
    ck_assert(type_is_unsigned_elementary(t_u32));
    ck_assert(!type_is_unsigned_elementary(t_i64));
    ck_assert(type_is_unsigned_elementary(t_u64));
    ck_assert(!type_is_unsigned_elementary(t_f32));
    ck_assert(!type_is_unsigned_elementary(t_f64));
    ck_assert(!type_is_unsigned_elementary(t_string));
    ck_assert(!type_is_unsigned_elementary(t_bool));
    ck_assert(!type_is_unsigned_elementary(t_nil));
} END_TEST

START_TEST (test_is_floating_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(!type_is_floating_elementary(t_i));
    ck_assert(!type_is_floating_elementary(t_u));
    ck_assert(!type_is_floating_elementary(t_i8));
    ck_assert(!type_is_floating_elementary(t_u8));
    ck_assert(!type_is_floating_elementary(t_i16));
    ck_assert(!type_is_floating_elementary(t_u16));
    ck_assert(!type_is_floating_elementary(t_i32));
    ck_assert(!type_is_floating_elementary(t_u32));
    ck_assert(!type_is_floating_elementary(t_i64));
    ck_assert(!type_is_floating_elementary(t_u64));
    ck_assert(type_is_floating_elementary(t_f32));
    ck_assert(type_is_floating_elementary(t_f64));
    ck_assert(!type_is_floating_elementary(t_string));
    ck_assert(!type_is_floating_elementary(t_bool));
    ck_assert(!type_is_floating_elementary(t_nil));
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
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    static const struct RFstring id_b =  RF_STRING_STATIC_INIT("b");
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_i64,
                                                                      t_leaf_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_nil,
                                                                      t_u32);

    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_leaf_i64, t_leaf_f64,
                                             t_func_1, t_foo };
    ck_assert_type_set_equal(expected_types, d->front.analyzer);
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

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    static const struct RFstring id_b =  RF_STRING_STATIC_INIT("b");
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_i64,
                                                                      t_leaf_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_prod_1,
                                                                      t_u32);

    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);
    struct type *t_boo = testsupport_analyzer_type_create_defined(&id_boo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_leaf_i64, t_leaf_f64,
                                             t_func_1, t_foo, t_boo};
    ck_assert_type_set_equal(expected_types, d->front.analyzer);
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

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_i64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_leaf_i8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_leaf_f32 = testsupport_analyzer_type_create_leaf(&id_d, t_f32);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
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
                                             t_leaf_f32, t_leaf_string, t_prod_1,
                                             t_prod_2, t_prod_3, t_prod_4,
                                             t_foo};
    ck_assert_type_set_equal(expected_types, d->front.analyzer);
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

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_leaf_ai64 = testsupport_analyzer_type_create_leaf(&id_a, t_i64);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_leaf_bf64 = testsupport_analyzer_type_create_leaf(&id_b, t_f64);

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_leaf_ai8 = testsupport_analyzer_type_create_leaf(&id_a, t_i8);
    struct type *t_leaf_ci8 = testsupport_analyzer_type_create_leaf(&id_c, t_i8);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
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
                                             t_foo, t_bar, t_foobar, t_prod_3
    };
    ck_assert_type_set_equal(expected_types, d->front.analyzer);
} END_TEST

START_TEST(test_determine_block_type1) {
    struct ast_node *block;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "d:f64 = 3.14 * 0.14\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    block = ast_node_get_child(front_testdriver_get_ast_root(d), 0);
    ck_assert_msg(block, "Block should be the first child of the root");
    const struct type *block_type = ast_node_get_type(block, AST_TYPERETR_DEFAULT);
    ck_assert_msg(block_type, "Block should have a type");
    ck_assert_msg(type_compare(block_type, t_f64, TYPECMP_IDENTICAL),
                  "Expected the block's type to be an f64");
} END_TEST

START_TEST(test_determine_block_type2) {
    struct ast_node *block;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string}\n"
        "{\n"
        "a:foo\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert_typecheck_ok(d, true);

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    static const struct RFstring id_a = RF_STRING_STATIC_INIT("a");
    struct type *t_leaf_ai8 = testsupport_analyzer_type_create_leaf(&id_a, t_i8);

    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    static const struct RFstring id_b = RF_STRING_STATIC_INIT("b");
    struct type *t_leaf_bstring = testsupport_analyzer_type_create_leaf(&id_b, t_string);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_leaf_ai8,
                                                                      t_leaf_bstring);
    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    block = ast_node_get_child(front_testdriver_get_ast_root(d), 1);
    ck_assert_msg(block, "Block should be the second child of the root");
    const struct type *block_type = ast_node_get_type(block, AST_TYPERETR_DEFAULT);
    ck_assert_msg(block_type, "Block should have a type");
    ck_assert_msg(type_compare(block_type, t_foo, TYPECMP_IDENTICAL),
                  "Expected the block's type to be an f64");
} END_TEST

Suite *types_suite_create(void)
{
    Suite *s = suite_create("types");

    TCase *st1 = tcase_create("types_general_tests");
    tcase_add_checked_fixture(st1, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_to_str);
    tcase_add_test(st1, test_type_comparison_identical);
    tcase_add_test(st1, test_type_comparison_for_sum_fncall);
    tcase_add_test(st1, test_type_comparison_for_sum_fncall_with_conversion);

    TCase *st2 = tcase_create("types_getter_tests");
    tcase_add_checked_fixture(st2, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st2, test_elementary_get_category);
    tcase_add_test(st2, test_is_signed_elementary);
    tcase_add_test(st2, test_is_unsigned_elementary);
    tcase_add_test(st2, test_is_floating_elementary);

    TCase *st3 = tcase_create("types_management_tests");
    tcase_add_checked_fixture(st3, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st3, test_composite_types_list_population);
    tcase_add_test(st3, test_composite_types_list_population2);
    tcase_add_test(st3, test_composite_types_list_population3);
    tcase_add_test(st3, test_composite_types_list_population4);

    TCase *st4 = tcase_create("type_determination");
    tcase_add_checked_fixture(st4, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st4, test_determine_block_type1);
    tcase_add_test(st4, test_determine_block_type2);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    suite_add_tcase(s, st3);
    suite_add_tcase(s, st4);
    return s;
}
