#include "testsupport.h"

#include <refu.h>

#include <check.h>

void setup_base_tests()
{
    ck_assert_msg(rf_init("refuclib.log", 0, LOG_DEBUG),
                  "Failed to initialize refu library");
}
void teardown_base_tests()
{
    rf_deinit();
}
