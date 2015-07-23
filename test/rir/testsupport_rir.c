#include "testsupport_rir.h"

#include <ir/rir_types_list.h>
#include <ir/rir.h>

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
    darray_init(d->rir_types);
    return true;
}

static void rir_testdriver_deinit(struct rir_testdriver *d)
{
    struct rir_type **t;

    // free the driver's own types
    darray_foreach(t, d->rir_types) {
        rir_type_destroy(*t);
    }
    darray_free(d->rir_types);
}

void setup_rir_tests_no_stdlib()
{
    setup_analyzer_tests_no_stdlib();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void setup_rir_tests_with_filelog()
{
    setup_analyzer_tests_with_filelog();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void setup_rir_tests_no_source()
{
    const struct RFstring s = RF_STRING_STATIC_INIT("");
    setup_analyzer_tests_no_stdlib();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
    front_testdriver_new_main_source(&s);
    testsupport_analyzer_prepare();
}

void teardown_rir_tests()
{
    rir_testdriver_deinit(&i_rir_test_driver_);
    teardown_analyzer_tests();
}

// this is a function only used in testing for convenience, in order to add a
// rir type always before all of its dependencies in the testing rir types list
// so that no problem happens during destruction.
// (This should not be a problem in production code)
static bool rir_type_is_subtype_of_other(struct rir_type *t, struct rir_type *other);
bool rir_type_is_subtype_of_other(struct rir_type *t, struct rir_type *other)
{
    struct rir_type **subtype;
    darray_foreach(subtype, other->subtypes) {
        if (*subtype == t) {
            return true;
        }
        if (darray_size((*subtype)->subtypes) != 0) {
            if (rir_type_is_subtype_of_other(t, *subtype)) {
                return true;
            }
        }
    }
    return false;
}

static void rir_testdriver_add_type(struct rir_type *type,
                                    const char* filename,
                                    unsigned int line)
{
    struct rir_type **subtype;
    struct rir_testdriver *d = get_rir_testdriver();
    // just a check that the type is not already added
    darray_foreach(subtype, d->rir_types) {
    RFS_PUSH();
        ck_assert_msg(*subtype != type,
                      "Attempted to re-add rir type ["RF_STR_PF_FMT"] in the "
                      "rir types list of the test driver from %s:%u",
                      RF_STR_PF_ARG(rir_type_str_or_die(type)),
                      filename,
                      line);
    RFS_POP();
    }

    // if adding type depends on any other type on the list add it before all
    // @look rir_type_is_subtype_of_other()
    darray_foreach(subtype, d->rir_types) {
        if (rir_type_is_subtype_of_other(*subtype, type)) {
            darray_prepend(d->rir_types, type);
            return;
        }
    }

    // else just append at the end
    darray_append(d->rir_types, type);
}

struct rir_type *i_testsupport_rir_type_create(enum rir_type_category category,
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
        rir_testdriver_add_type(ret, filename, line);
    }
    return ret;
}

void i_testsupport_rir_type_add_subtype(struct rir_type *type,
                                        struct rir_type *subtype,
                                        bool add_to_drivers_list,
                                        const char* filename,
                                        unsigned int line)
{
    darray_append(type->subtypes, subtype);
    if (add_to_drivers_list) {
        rir_testdriver_add_type(type, filename, line);
    }
}

bool i_rir_testdriver_compare_lists(struct rir_type **expected_types,
                                    unsigned int expected_num,
                                    const char* filename,
                                    unsigned int line)
{
    unsigned int i;
    unsigned int count = 0;
    struct rir_type *t;
    bool found;
    rir_types_list_for_each(front_testdriver_module()->rir->rir_types_list, t) {
        RFS_PUSH();
        found = false;
        for (i = 0; i < expected_num; ++i) {
            if (rir_type_equals(t, expected_types[i], RIR_TYPECMP_NAMES)) {
                found = true;
                break;
            }
        }
        ck_assert_msg(found, "Encountered rir type ["RF_STR_PF_FMT"] was not found in the "
                      "expected types list from  %s:%u",
                      RF_STR_PF_ARG(rir_type_str_or_die(t)),
                      filename,
                      line);
        count ++;
        RFS_POP();
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


i_INLINE_INS struct rf_objset_type *testsupport_rir_typeset(const struct rir_testdriver *d);
