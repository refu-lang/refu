#include "testsupport_rir.h"

#include <ir/rir.h>
#include <ir/elements.h>

static struct rir_testdriver i_rir_test_driver_;

struct rir_testdriver *get_rir_testdriver()
{
    return &i_rir_test_driver_;
}

static bool rir_testdriver_init(struct rir_testdriver *d,
                                struct front_testdriver *front_driver,
                                struct analyzer_testdriver *analyzer_driver)
{
    d->front_driver = front_driver;
    d->analyzer_driver = analyzer_driver;
    d->rir = NULL;
    d->module = NULL;
    darray_init(d->rir_types);
    return true;
}

static void rir_testdriver_deinit(struct rir_testdriver *d)
{
    struct rir_type **t;
    if (d->rir) {
        rir_destroy(d->rir);
    }
    if (d->module) {
        rir_module_destroy(d->module);
    }

    // free the driver's own types
    darray_foreach(t, d->rir_types) {
        rir_type_destroy(*t);
    }
    darray_free(d->rir_types);
}

void setup_rir_tests()
{
    setup_front_tests();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void setup_rir_tests_with_filelog()
{
    setup_front_tests_with_file_log();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void teardown_rir_tests()
{
    rir_testdriver_deinit(&i_rir_test_driver_);
    teardown_front_tests();
}

void rir_testdriver_assign(struct rir_testdriver *d,
                           const struct RFstring *s)
{
    front_testdriver_assign(d->front_driver, s);
}

bool rir_testdriver_process(struct rir_testdriver *d)
{
    d->rir = rir_create(d->front_driver->front.analyzer);
    if (!d->rir) {
        return false;
    }
    d->module = rir_process(d->rir);
    if (!d->module) {
        return false;
    }

    return true;
}

struct rir_type *testsupport_rir_type_create(struct rir_testdriver *d,
                                             enum rir_type_category category,
                                             const struct RFstring *name)
{
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->category = category;
    darray_init(ret->subtypes);
    ret->name = name;
    darray_append(d->rir_types, ret);
    ret->indexed = true;
    return ret;
}

void testsupport_rir_type_add_subtype(struct rir_type *type, struct rir_type *subtype)
{
    darray_append(type->subtypes, subtype);
}

bool i_rir_testdriver_compare_lists(struct rir_testdriver *d,
                                    struct rir_type **expected_types,
                                    unsigned int expected_num,
                                    const char* filename,
                                    unsigned int line)
{
    unsigned int i;
    unsigned int count = 1;
    struct rir_type *t;
    bool found;
    rf_ilist_for_each(&d->rir->rir_types, t, ln) {
        found = false;
        for (i = 0; i < expected_num; ++i) {
            if (rir_type_equals(t, expected_types[i])) {
                found = true;
                break;
            }
        }
        ck_assert_msg(found, "Encountered rir type ["RF_STR_PF_FMT"] was not found in the "
                      "expected types list from  %s:%u",
                      RF_STR_PF_ARG(rir_type_str(t)), filename, line);
        count ++;
    }

    ck_assert_msg(expected_num == count, "Number of expected rir types (%u) does not "
                  "match the number of created types (%u) from %s:%u", expected_num,
                  count, filename, line);
    return true;
}
