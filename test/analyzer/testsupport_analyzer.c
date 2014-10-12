#include "testsupport_analyzer.h"
#include "../testsupport_front.h"

#include <refu.h>
#include <check.h>

#include <ast/ast.h>

static struct analyzer_testdriver i_analyzer_test_driver_;
static bool analyzer_testdriver_init(struct analyzer_testdriver *d)
{
    //TODO: ?
    return true;
}

static void analyzer_testdriver_deinit(struct analyzer_testdriver *d)
{
    //TODO: ?
}

struct analyzer_testdriver *get_analyzer_testdriver()
{
    return &i_analyzer_test_driver_;
}


void setup_analyzer_tests()
{
    setup_front_tests();
    ck_assert_msg(analyzer_testdriver_init(&i_analyzer_test_driver_),
                  "Failed to initialize the analyzer test driver");
}

void teardown_analyzer_tests()
{
    teardown_front_tests();
    analyzer_testdriver_deinit(&i_analyzer_test_driver_);
}
