#ifndef LFR_TESTSUPPORT_RIR_H
#define LFR_TESTSUPPORT_RIR_H

// NOTE: This test support file can slowly go away
//       RIR is now just a final state of the analyzer's AST

#include "../testsupport_front.h"
#include "../analyzer/testsupport_analyzer.h"
#include <ir/rir_type.h>
#include <Definitions/inline.h>

struct rir_testdriver {
    struct front_testdriver *front_driver;
    struct analyzer_testdriver *analyzer_driver;
    //! A buffer of rir types for quick type checks
    //! and easy freeing at test teardown
    struct {darray(struct rir_type*);} rir_types;
};
struct rir_testdriver *get_rir_testdriver();

void setup_rir_tests();
void setup_rir_tests_with_filelog();
void teardown_rir_tests();

void rir_testdriver_assign(struct rir_testdriver *d,
                           const struct RFstring *s);

bool rir_testdriver_process(struct rir_testdriver *d);

#define testsupport_rir_process(driver_, with_stdlib_)                  \
    do {                                                                \
        testsupport_have_stdlib((driver_)->front_driver, with_stdlib_); \
        testsupport_scan_and_parse((driver_)->front_driver);            \
        if (!analyzer_analyze_file(((driver_)->front_driver)->front.analyzer, \
                                   ((driver_)->front_driver)->front.parser, \
                                   ((driver_)->front_driver)->stdlib)) {  \
            testsupport_show_front_errors((driver_)->front_driver, "Typechecking failed"); \
        }                                                               \
        ck_assert_msg(rir_testdriver_process((driver_)), "Failed to create the refu intermediate format"); \
    } while (0)

struct rir_type *i_testsupport_rir_type_create(struct rir_testdriver *d,
                                               enum rir_type_category category,
                                               const struct RFstring *name,
                                               bool add_to_drivers_list,
                                               const char* filename,
                                               unsigned int line);
#define testsupport_rir_type_create(driver_, category_, name_, add_to_drivers_list_) \
    i_testsupport_rir_type_create(driver_, category_, name_,            \
                                  add_to_drivers_list_, __FILE__, __LINE__)
void i_testsupport_rir_type_add_subtype(struct rir_testdriver *d,
                                        struct rir_type *type,
                                        struct rir_type *subtype,
                                        bool add_to_drivers_list,
                                        const char* filename,
                                        unsigned int line);
#define testsupport_rir_type_add_subtype(driver_, type_, subtype_, add_to_drivers_list) \
    i_testsupport_rir_type_add_subtype(driver_, type_, subtype_,        \
                                       add_to_drivers_list, __FILE__, __LINE__)

#define rir_testdriver_compare_lists(driver_, expected_types_)          \
    i_rir_testdriver_compare_lists(driver_, expected_types_,            \
                                   sizeof(expected_types)/sizeof(struct rir_type*), \
                                   __FILE__, __LINE__);
bool i_rir_testdriver_compare_lists(struct rir_testdriver *d,
                                    struct rir_type **expected_types,
                                    unsigned int expected_num,
                                    const char* filename,
                                    unsigned int line);

i_INLINE_DECL struct rf_objset_type *testsupport_rir_typeset(const struct rir_testdriver *d)
{
    return d->front_driver->front.analyzer->types_set;
}
#endif
