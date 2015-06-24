#ifndef LFR_TESTSUPPORT_END_TO_END_H
#define LFR_TESTSUPPORT_END_TO_END_H

#include <compiler.h>

struct RFstring;

struct end_to_end_driver {
    struct compiler *compiler;
    struct RFstring *file_name;
};

struct end_to_end_driver *get_end_to_end_driver();

void setup_end_to_end_tests();
void teardown_end_to_end_tests();

bool end_to_end_driver_create_file(struct end_to_end_driver *d,
                                   const char *file_name,
                                   const struct RFstring *s);

bool end_to_end_driver_compile(struct end_to_end_driver *d, char *args);

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value,
                           const struct RFstring *output);

#define i_ck_end_to_end_run_with_arguments0(i_driver_, i_filename_, i_string_, i_expected_ret_, i_stdout_) \
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_driver_create_file(i_driver_, i_filename_, i_string_), \
                      "Could not create input file for the test driver"); \
        ck_assert_msg(end_to_end_driver_compile(i_driver_, i_filename_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_driver_run(i_driver_, &actual_ret, i_stdout_),   \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)

#define i_ck_end_to_end_run_with_arguments1(i_driver_, i_filename_, i_string_, i_expected_ret_, i_stdout_, i_arguments_) \
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_driver_create_file(i_driver_, i_filename_, i_string_), \
                      "Could not create input file for the test driver"); \
        ck_assert_msg(end_to_end_driver_compile(i_driver_, i_arguments_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_driver_run(i_driver_, &actual_ret, i_stdout_),   \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)

#define i_ck_end_to_end_run_with_stdout1(...)                                       \
    RF_SELECT_FUNC_IF_NARGGT2(i_ck_end_to_end_run_with_arguments, 5, __VA_ARGS__)

#define i_ck_end_to_end_run_with_stdout0(i_driver_, i_filename_, i_string_, i_expected_ret_) \
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_driver_create_file(i_driver_, i_filename_, i_string_), \
                      "Could not create input file for the test driver"); \
        ck_assert_msg(end_to_end_driver_compile(i_driver_, i_filename_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_driver_run(i_driver_, &actual_ret, NULL),   \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)


/**
 * Runs end to end tests
 *
 * Some argument's can be ommitted and then the default value will be used
 *
 * @param           i_driver_                 The end to end test driver
 * @param           i_filename_               The filename to create for the test source
 * @param           i_string_                 The source of the program to test in an RFstring
 * @param           i_expected_ret            The program's expected return value
 * @param[optional] i_stdout_                 The expected stdout of the program in
 *                                            an RFstring or NULL if there should be no stdout
 *                                            or we don't care to check. Default is NULL.
 * @param[optional] i_arguments               The arguments to provide to the compiler as a
 *                                            a cstring. The default is only the filename.
 */
#define ck_end_to_end_run(...)                                      \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_end_to_end_run_with_stdout, 4, __VA_ARGS__)


#endif
