#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/core.h>
#include <ast/ast.h>
#include <ast/function.h>

#include "testsupport_rir.h"

#include CLIB_TEST_HELPERS

struct rir_misctest_driver {
    struct {darray(struct rir_type*);} test_rir_types;
};
struct rir_misctest_driver g_rir_misctest_driver;

static void setup_rir_misc_tests()
{
    darray_init(g_rir_misctest_driver.test_rir_types);
}

static void teardown_rir_misc_tests()
{
    struct rir_type **t;
    darray_foreach(t, g_rir_misctest_driver.test_rir_types) {
        if ((*t)->category == RIR_TYPE_ELEMENTARY) {
            free(*t);
        } else if ((*t)->category == RIR_TYPE_COMPOSITE) {
            struct rir_typedef *tdef = (struct rir_typedef*)(*t)->tdef;
            rf_string_deinit(&tdef->name);
            free(tdef);
            free((*t));
        } else {
            RF_CRITICAL_FAIL("Invalid rir type category");
        }
    }
    darray_free(g_rir_misctest_driver.test_rir_types);
}

static struct rir_type *test_create_rir_elementary_type(
    enum elementary_type etype,
    bool is_pointer)
{
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_type_elem_init(ret, etype, is_pointer);
    darray_append(g_rir_misctest_driver.test_rir_types, ret);
    return ret;
}

// creates a "fake/empty" composite type
static struct rir_type *test_create_rir_composite_type(
    const char *name,
    bool is_pointer)
{
    struct rir_type *ret;
    struct rir_typedef *tdef;
    RF_MALLOC(tdef, sizeof(*tdef), return NULL);
    if (!rf_string_init(&tdef->name, name)) {
        free(tdef);
        return NULL;
    }
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_type_comp_init(ret, tdef, is_pointer);
    darray_append(g_rir_misctest_driver.test_rir_types, ret);
    return ret;
}



START_TEST (test_rir_type_map1) {
    struct rirtype_strmap map;
    struct rir_type *t1 = test_create_rir_elementary_type(
        ELEMENTARY_TYPE_STRING,
        false
    );
    static const struct RFstring s = RF_STRING_STATIC_INIT("string");
    strmap_init(&map);

    ck_assert(!rirtype_strmap_get(&map, &s));
    ck_assert(rirtype_strmap_add(&map, &s, t1));
    ck_assert(t1 == rirtype_strmap_get(&map, &s));

    rirtype_strmap_free(&map);
} END_TEST

START_TEST (test_rir_type_map2) {
    struct rirtype_strmap map;
    static const struct RFstring s1 = RF_STRING_STATIC_INIT("person");
    static const struct RFstring s2 = RF_STRING_STATIC_INIT("person*");
    struct rir_type *t1 = test_create_rir_composite_type("person", false);
    struct rir_type *t2 = test_create_rir_composite_type("person", true);
    strmap_init(&map);

    ck_assert(!rirtype_strmap_get(&map, &s1));
    ck_assert(rirtype_strmap_add(&map, &s2, t2));
    ck_assert(t2 == rirtype_strmap_get(&map, &s2));
    ck_assert(!rirtype_strmap_get(&map, &s1));
    ck_assert(rirtype_strmap_add(&map, &s1, t1));
    ck_assert(t1 == rirtype_strmap_get(&map, &s1));

    rirtype_strmap_free(&map);
} END_TEST

Suite *rir_misctest_suite_create(void)
{
    Suite *s = suite_create("rir_miscellaneous_tests");

    TCase *tc1 = tcase_create("rir_type_strmap");
    tcase_add_checked_fixture(tc1, setup_rir_misc_tests, teardown_rir_misc_tests);
    tcase_add_test(tc1, test_rir_type_map1);
    tcase_add_test(tc1, test_rir_type_map2);

    suite_add_tcase(s, tc1);

    return s;
}
