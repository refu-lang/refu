#include "testsupport_rir.h"

#include <ir/rir.h>
#include <ir/elements.h>
#include <ir/rir_types_list.h>

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

static void rir_testdriver_add_type(struct rir_testdriver *d,
                                    struct rir_type *type,
                                    const char* filename,
                                    unsigned int line)
{
    struct rir_type **subtype;
    // just a check that the type is not already added
    darray_foreach(subtype, d->rir_types) {
    RFS_push();
        struct RFstring *type_s;
        ck_assert(rir_type_str(&type_s, type));
        ck_assert_msg(*subtype != type,
            "Attempted to re-add rir type ["RF_STR_PF_FMT"] in the "
            "rir types list of the test driver from %s:%u",
            RF_STR_PF_ARG(type_s), filename, line);
    RFS_pop();
    }

    // if adding type depends on any other type on the list add it before all
    darray_foreach(subtype, d->rir_types) {
        if (rir_type_is_subtype_of_other(*subtype, type)) {
            darray_prepend(d->rir_types, type);
            return;
        }
    }

    // else just append at the end
    darray_append(d->rir_types, type);
}

struct rir_type *i_testsupport_rir_type_create(struct rir_testdriver *d,
                                               enum rir_type_category category,
                                               const struct RFstring *name,
                                               bool add_to_drivers_list,
                                               const char* filename,
                                               unsigned int line)
{
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->category = category;
    darray_init(ret->subtypes);
    ret->name = name;
    ret->indexed = true;
    if (add_to_drivers_list) {
        rir_testdriver_add_type(d, ret, filename, line);
    }
    return ret;
}

void i_testsupport_rir_type_add_subtype(struct rir_testdriver *d,
                                        struct rir_type *type,
                                        struct rir_type *subtype,
                                        bool add_to_drivers_list,
                                        const char* filename,
                                        unsigned int line)
{
    darray_append(type->subtypes, subtype);
    if (add_to_drivers_list) {
        rir_testdriver_add_type(d, type, filename, line);
    }
}

bool i_rir_testdriver_compare_lists(struct rir_testdriver *d,
                                    struct rir_type **expected_types,
                                    unsigned int expected_num,
                                    const char* filename,
                                    unsigned int line)
{
    unsigned int i;
    unsigned int count = 0;
    struct rir_type *t;
    struct RFstring *type_s;
    bool found;
    rir_types_list_for_each(&d->rir->rir_types_list, t) {
    RFS_push();
        found = false;
        for (i = 0; i < expected_num; ++i) {
            if (rir_type_equals(t, expected_types[i])) {
                found = true;
                break;
            }
        }
        ck_assert(rir_type_str(&type_s, t));
        ck_assert_msg(found, "Encountered rir type ["RF_STR_PF_FMT"] was not found in the "
                      "expected types list from  %s:%u",
                      RF_STR_PF_ARG(type_s), filename, line);
        count ++;
    RFS_pop();
    }
    ck_assert_msg(expected_num == count,
        "Number of expected rir types (%u) does not "
        "match the number of created types (%u) from %s:%u",
        expected_num,
        count,
        filename,
        line);
    return true;
}
