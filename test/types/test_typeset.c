#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <types/type.h>
#include <types/type_operators.h>
#include <types/type_elementary.h>
#include <ast/type.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "../analyzer/testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

static void ck_assert_type_set_equal_impl(const struct type **expected_types,
                                          size_t expected_types_size,
                                          struct module *m,
                                          const char *filename,
                                          unsigned int line)
{
    struct type *t;
    struct rf_objset_iter it;
    unsigned int i;
    bool found;
    unsigned int found_types_size = 0;
    bool *found_indexes;
    ck_assert_msg(m->types_set, "Module's type set was not created");
    RF_CALLOC(found_indexes, expected_types_size, sizeof(*found_indexes), return);

    rf_objset_foreach(m->types_set, &it, t) {
        found = false;
        for (i = 0; i < expected_types_size; ++i) {
            if (type_compare(t, expected_types[i], TYPECMP_IDENTICAL)) {
                found = true;

                RFS_PUSH();
                ck_assert_msg(!found_indexes[i],
                              "Found a duplicate type entry in the list. Type: "
                              RF_STR_PF_FMT,
                              RF_STR_PF_ARG(type_str_or_die(expected_types[i], TSTR_DEFAULT)));
                found_indexes[i] = true;
                RFS_POP();
                break;
            }
        }
        if (!found) {
            ck_abort_msg("Did not manage to find type "RF_STR_PF_FMT" "
                         "in the expected types list from %s:%u",
                         RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)),
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
                      RF_STR_PF_ARG(type_str_or_die(expected_types[i], TSTR_DEFAULT)),
                      filename, line);
        RFS_POP();
    }
    free(found_indexes);
}

#define ck_assert_type_set_equal(i_expected_types_, i_module_)      \
    ck_assert_type_set_equal_impl(i_expected_types_,                  \
                                  sizeof(i_expected_types_) / sizeof(struct type*), \
                                  i_module_, __FILE__, __LINE__)

#define ck_assert_type_set_can_be_ordered_properly(i_typeset_)          \
    do {                                                                \
        struct type_arr arr;                                            \
        ck_assert(typeset_to_ordered_array(i_typeset_, &arr));          \
        struct type **t;                                                \
        struct type **t2;                                               \
        darray_foreach(t, arr) {                                        \
            bool after_t = false;                                       \
            darray_foreach(t2, arr) {                                   \
                if (*t2 == *t) {                                        \
                    after_t = true;                                     \
                    continue;                                           \
                }                                                       \
                if (after_t && type_is_childof(*t2, *t) != -1) {        \
                    ck_abort_msg(                                       \
                        "Typeset was not ordered properly. Type "       \
                        RF_STR_PF_FMT "depends on type "                \
                        RF_STR_PF_FMT".",                               \
                        RF_STR_PF_ARG(type_str_or_die(*t, TSTR_DEFAULT)), \
                        RF_STR_PF_ARG(type_str_or_die(*t2, TSTR_DEFAULT)) \
                    );                                                  \
                }                                                       \
            }                                                           \
        }                                                               \
        darray_free(arr);                                               \
    } while(0)

START_TEST(test_type_set_population) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}"
        "fn do_something() -> u32\n"
        "{\n"
        "return 15"
        "}\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT, t_i64, t_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_nil,
                                                                      t_u32);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_func_1, t_foo };
    ck_assert_type_set_equal(expected_types, front_testdriver_module());
} END_TEST

START_TEST(test_type_set_population2) {
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

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT, t_i64, t_f64);

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_func_1 = testsupport_analyzer_type_create_operator(TYPEOP_IMPLICATION,
                                                                      t_prod_1,
                                                                      t_u32);

    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);
    struct type *t_boo = testsupport_analyzer_type_create_defined(&id_boo, t_prod_1);

    const struct type *expected_types [] = { t_prod_1, t_func_1, t_foo, t_boo};
    ck_assert_type_set_equal(expected_types, front_testdriver_module());
} END_TEST

START_TEST(test_type_set_population3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64, c:i8, d:f32, e:string}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);


    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);


    struct type *t_prod = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                    t_i64,
                                                                    t_f64,
                                                                    t_i8,
                                                                    t_f32,
                                                                    t_string);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod);

    const struct type *expected_types [] = { t_prod, t_foo};
    ck_assert_type_set_equal(expected_types, front_testdriver_module());
} END_TEST

START_TEST(test_type_set_population4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i64, b:f64}\n"
        "type bar {a:i8, b:string}\n"
        "type foobar {a:i64, b:f64 | c:i8, d:string}\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    static const struct RFstring id_bar = RF_STRING_STATIC_INIT("bar");
    static const struct RFstring id_foobar = RF_STRING_STATIC_INIT("foobar");

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_i64,
                                                                      t_f64);
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    struct type *t_prod_2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_i8,
                                                                      t_string);
    struct type *t_bar = testsupport_analyzer_type_create_defined(&id_bar, t_prod_2);


    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                      t_prod_1,
                                                                      t_prod_2);
    struct type *t_foobar = testsupport_analyzer_type_create_defined(&id_foobar, t_sum_1);

    const struct type *expected_types [] = { t_prod_1, t_prod_2, t_sum_1,
                                             t_foo, t_bar, t_foobar
    };
    ck_assert_type_set_equal(expected_types, front_testdriver_module());
} END_TEST

START_TEST(test_types_set_has_uid1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i8| d:f32 }\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_i8,
                                                                   t_f32);

    size_t uid = type_get_uid(t_sum);
    ck_assert(type_objset_has_uid(front_testdriver_module()->types_set, uid));
} END_TEST

START_TEST(test_types_set_has_uid2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i8, b:string | c:f32, d:u64, e:u8 }\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_prod1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_i8,
                                                                     t_string);
    struct type *t_prod2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_f32,
                                                                     t_u64,
                                                                     t_u8);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_prod1,
                                                                   t_prod2);

    size_t uid = type_get_uid(t_sum);
    ck_assert(type_objset_has_uid(front_testdriver_module()->types_set, uid));
} END_TEST

START_TEST(test_types_set_has_string) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i8, b:string | c:f32, d:u64, e:u8 }\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    const struct RFstring s1 = RF_STRING_STATIC_INIT("i8,string|f32,u64,u8");
    const struct RFstring s2 = RF_STRING_STATIC_INIT("i8, string|f32,u64,u8");
    ck_assert(type_objset_has_string(front_testdriver_module()->types_set, &s1));
    ck_assert(!type_objset_has_string(front_testdriver_module()->types_set, &s2));
} END_TEST

START_TEST(test_typeset_to_ordered_array1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i8, b:string | c:f32, d:u64, e:u8 }\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();
    ck_assert_type_set_can_be_ordered_properly(front_testdriver_module()->types_set);
} END_TEST

START_TEST(test_typeset_to_ordered_array2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:u64, b:f32 | c:i64, d:u64 | e:i8, f:u64 }\n"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();
    ck_assert_type_set_can_be_ordered_properly(front_testdriver_module()->types_set);
} END_TEST

Suite *type_set_suite_create(void)
{
    Suite *s = suite_create("type_set");

    TCase *st1 = tcase_create("types_set_population");
    tcase_add_checked_fixture(st1, setup_analyzer_tests_no_stdlib, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_set_population);
    tcase_add_test(st1, test_type_set_population2);
    tcase_add_test(st1, test_type_set_population3);
    tcase_add_test(st1, test_type_set_population4);

    TCase *st2 = tcase_create("types_set_has_uid");
    tcase_add_checked_fixture(st2, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st2, test_types_set_has_uid1);
    tcase_add_test(st2, test_types_set_has_uid2);

    TCase *st3 = tcase_create("types_set_has_string");
    tcase_add_checked_fixture(st3, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st3, test_types_set_has_string);

    TCase *st4 = tcase_create("types_set_to_ordered_array");
    tcase_add_checked_fixture(st4, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st4, test_typeset_to_ordered_array1);
    tcase_add_test(st4, test_typeset_to_ordered_array2);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    suite_add_tcase(s, st3);
    suite_add_tcase(s, st4);
    return s;
}
