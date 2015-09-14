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

void setup_rir_tests_no_stdlib();
void setup_rir_tests_no_source();
void setup_rir_tests_with_filelog();
void teardown_rir_tests();

struct rir_type *i_testsupport_rir_type_create(enum rir_type_category category,
                                               const struct RFstring *name,
                                               bool add_to_drivers_list,
                                               const char* filename,
                                               unsigned int line);
#define testsupport_rir_type_create(category_, name_, add_to_drivers_list_) \
    i_testsupport_rir_type_create(category_, name_, add_to_drivers_list_, __FILE__, __LINE__)
void i_testsupport_rir_type_add_subtype(struct rir_type *type,
                                        struct rir_type *subtype,
                                        bool add_to_drivers_list,
                                        const char* filename,
                                        unsigned int line);
#define testsupport_rir_type_add_subtype(type_, subtype_, add_to_drivers_list) \
    i_testsupport_rir_type_add_subtype(type_, subtype_, add_to_drivers_list, __FILE__, __LINE__)

#define rir_testdriver_compare_lists(expected_types_)                   \
    i_rir_testdriver_compare_lists(expected_types_, \
                                   sizeof(expected_types)/sizeof(struct rir_type*), \
                                   __FILE__, __LINE__);
bool i_rir_testdriver_compare_lists(struct rir_type **expected_types,
                                    unsigned int expected_num,
                                    const char* filename,
                                    unsigned int line);

#define ck_assert_rir_type_equals_type(i_rirt_, i_t_, i_str_)           \
    do {                                                                \
        ck_assert_msg(rir_type_equals_type(i_rirt_, i_t_, i_str_),      \
                      "Expected rir type \""RF_STR_PF_FMT"\" to be equal " \
                      "to normal type \""RF_STR_PF_FMT"\".",            \
                      RF_STR_PF_ARG(rir_type_str_or_die(i_rirt_)),      \
                      RF_STR_PF_ARG(type_str_or_die(i_t_, TSTR_DEFAULT)) \
        );                                                              \
    } while (0)

i_INLINE_DECL struct rf_objset_type *testsupport_rir_typeset(const struct rir_testdriver *d)
{
    return front_testdriver_module()->types_set;
}
#endif
