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

    struct type *t_u32 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_32);
    static const struct RFstring id_foo =  RF_STRING_STATIC_INIT("foo");
    struct type *t_leaf_u32 = testsupport_analyzer_type_create_leaf(&id_foo, t_u32);

    struct type *t_f64 = testsupport_analyzer_type_create_builtin(BUILTIN_FLOAT_64);
    static const struct RFstring id_boo =  RF_STRING_STATIC_INIT("boo");
    struct type *t_leaf_f64 = testsupport_analyzer_type_create_leaf(&id_boo, t_f64);

    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_leaf_u32,
                                                                     t_leaf_f64);

    RFS_buffer_push();
    ck_assert_rf_str_eq_cstr(type_str(t_u32), "u32");
    ck_assert_rf_str_eq_cstr(type_str(t_leaf_u32), "foo:u32");
    ck_assert_rf_str_eq_cstr(type_str(t_prod_1), "foo:u32,boo:f64");
    RFS_buffer_pop();
} END_TEST

Suite *types_suite_create(void)
{
    Suite *s = suite_create("types");

    TCase *st1 = tcase_create("types_general_tests");
    tcase_add_checked_fixture(st1, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_to_str);

    suite_add_tcase(s, st1);
    return s;
}
