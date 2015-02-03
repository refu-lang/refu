#ifndef LFR_TESTSUPPORT_END_TO_END_H
#define LFR_TESTSUPPORT_END_TO_END_H

#include <compiler.h>

struct RFstring;

struct end_to_end_driver {
    struct compiler compiler;
    struct RFstring *file_name;
};

struct end_to_end_driver *get_end_to_end_driver();

void setup_end_to_end_tests();
void teardown_end_to_end_tests();

bool end_to_end_driver_create_file(struct end_to_end_driver *d,
                                   const char *file_name,
                                   const struct RFstring *s);

bool end_to_end_driver_compile(struct end_to_end_driver *d, char *args);

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value);

#define i_ck_end_to_end_run1(i_driver_, i_filename_, i_string_, i_expected_ret_, i_arguments_) \
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_driver_create_file(i_driver_, i_filename_, i_string_), \
                      "Could not create input file for the test driver"); \
        ck_assert_msg(end_to_end_driver_compile(i_driver_, i_arguments_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_driver_run(i_driver_, &actual_ret),    \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)

#define i_ck_end_to_end_run0(i_driver_, i_filename_, i_string_, i_expected_ret_) \
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_driver_create_file(i_driver_, i_filename_, i_string_), \
                      "Could not create input file for the test driver"); \
        ck_assert_msg(end_to_end_driver_compile(i_driver_, i_filename_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_driver_run(i_driver_, &actual_ret),    \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)

#define ck_end_to_end_run(...)                                      \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_end_to_end_run, 4, __VA_ARGS__)


#endif
