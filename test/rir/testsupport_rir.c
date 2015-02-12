#include "testsupport_rir.h"

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
    d->rir = NULL;
    d->module = NULL;
    return true;
}

static void rir_testdriver_deinit(struct rir_testdriver *d)
{
    if (d->rir) {
        rir_destroy(d->rir);
    }
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
