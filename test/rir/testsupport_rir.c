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
    darray_init(d->target_rirs);
    return true;
}

static void rir_testdriver_deinit(struct rir_testdriver *d)
{
    // free the target rirs
    struct rir **r;
    darray_foreach(r, d->target_rirs) {
        rir_destroy(*r);
    }
    darray_free(d->target_rirs);
}

void setup_rir_tests()
{
    setup_analyzer_tests();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
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
    setup_analyzer_tests_no_stdlib();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
    front_testdriver_new_ast_main_source(rf_string_empty_get());
    testsupport_analyzer_prepare();
}

void teardown_rir_tests()
{
    rir_testdriver_deinit(&i_rir_test_driver_);
    teardown_analyzer_tests();
}

/* -- Functions that facilitate the specification of a target RIR -- */
i_INLINE_INS void testsupport_rir_set_curr_module(struct rir *r);
i_INLINE_INS void testsupport_rir_set_curr_fn(struct rir_fndef *fn);
i_INLINE_INS void testsupport_rir_set_curr_block(struct rir_block *block);

struct rir *testsupport_rir_add_module()
{
    struct rir_testdriver *tdr = get_rir_testdriver();
    struct rir *r = rir_create();
    if (!r) {
        return NULL;
    }
    darray_append(tdr->target_rirs, r);
    testsupport_rir_set_curr_module(r);
    return r;
}
