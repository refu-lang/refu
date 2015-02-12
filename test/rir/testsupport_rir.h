#ifndef LFR_TESTSUPPORT_RIR_H
#define LFR_TESTSUPPORT_RIR_H

#include "../testsupport_front.h"
#include "../analyzer/testsupport_analyzer.h"
#include <ir/rir.h>
#include <ir/rir_type.h>

struct rir_testdriver {
    struct front_testdriver *front_driver;
    struct analyzer_testdriver *analyzer_driver;

    struct rir *rir;
    struct rir_module *module;
};
struct rir_testdriver *get_rir_testdriver();

void setup_rir_tests();
void setup_rir_tests_with_filelog();
void teardown_rir_tests();

void rir_testdriver_assign(struct rir_testdriver *d,
                           const struct RFstring *s);

bool rir_testdriver_process();

#define testsupport_rir_process(driver_)                                \
    do {                                                                \
        testsupport_scan_and_parse((driver_)->front_driver);            \
        if (!analyzer_analyze_file(((driver_)->front_driver)->front.analyzer, ((driver_)->front_driver)->front.parser, true)) { \
            testsupport_show_front_errors((driver_)->front_driver, "Typechecking failed"); \
        }                                                               \
        ck_assert_msg(rir_testdriver_process((driver_)), "Failed to create the refu intermediate format"); \
    } while (0)

#endif
