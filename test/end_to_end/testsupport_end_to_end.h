#ifndef LFR_TESTSUPPORT_END_TO_END_H
#define LFR_TESTSUPPORT_END_TO_END_H

#include <compiler.h>

struct RFstring;

struct test_input_pair {
    struct RFstring filename;
    struct RFstring contents;
};

struct end_to_end_driver {
    struct compiler *compiler;
    struct RFstring *file_names;
    unsigned files_num;
};

struct end_to_end_driver *get_end_to_end_driver();

void setup_end_to_end_tests();
void teardown_end_to_end_tests();

bool end_to_end_driver_create_files(struct end_to_end_driver *d,
                                    struct test_input_pair *inputs,
                                    unsigned num);
i_INLINE_DECL bool end_to_end_create_files(struct test_input_pair *inputs, unsigned num)
{
    return end_to_end_driver_create_files(get_end_to_end_driver(), inputs, num);
}

bool end_to_end_driver_compile(struct end_to_end_driver *d,
                               const struct test_input_pair *inputs,
                               unsigned inputsn,
                               char *other_args);
i_INLINE_DECL bool end_to_end_compile(const struct test_input_pair *inputs,
                                      unsigned inputsn,
                                      char *other_args)
{
    return end_to_end_driver_compile(get_end_to_end_driver(), inputs, inputsn, other_args);
}

bool end_to_end_driver_run(struct end_to_end_driver *d,
                           int *ret_value,
                           const struct RFstring *output);
i_INLINE_DECL bool end_to_end_run(int *ret_value,
                                  const struct RFstring *output)
{
    return end_to_end_driver_run(get_end_to_end_driver(), ret_value, output);
}

#define TEST_DECL_SRC(i_name_, i_contents_) { RF_STRING_STATIC_INIT(i_name_), RF_STRING_STATIC_INIT(i_contents_) }
#define PASS_SRC_ARR(i_inputs_) (i_inputs_), (sizeof(i_inputs_) / sizeof(struct test_input_pair))


#define i_ck_end_to_end_run_impl(i_inputs_, i_expected_ret_, i_stdout_, i_arguments_)\
    do {                                                                \
        int actual_ret;                                                 \
        ck_assert_msg(end_to_end_create_files(PASS_SRC_ARR(i_inputs_)), \
                      "Could not create input file/s");                 \
        ck_assert_msg(end_to_end_compile(PASS_SRC_ARR(i_inputs_), i_arguments_), \
                      "Could not compile the input file");              \
        ck_assert_msg(end_to_end_run(&actual_ret, i_stdout_),   \
                      "Failed to execute driver's compiled result");    \
        ck_assert_msg(i_expected_ret_ == actual_ret, "Program return values do not match." \
                      "Expected %u but got %u", i_expected_ret_, actual_ret); \
    }while (0)

#define i_ck_end_to_end_run_with_arguments0(i_inputs_, i_expected_ret_, i_stdout_) \
    i_ck_end_to_end_run_impl(i_inputs_, i_expected_ret_, i_stdout_, NULL)

#define i_ck_end_to_end_run_with_arguments1(i_inputs_, i_expected_ret_, i_stdout_, i_arguments_) \
    i_ck_end_to_end_run_impl(i_inputs_, i_expected_ret_, i_stdout_, i_arguments)

#define i_ck_end_to_end_run_with_stdout1(...)                           \
    RF_SELECT_FUNC_IF_NARGGT2(i_ck_end_to_end_run_with_arguments, 3, __VA_ARGS__)

#define i_ck_end_to_end_run_with_stdout0(i_inputs_, i_expected_ret_)    \
    i_ck_end_to_end_run_impl(i_inputs_, i_expected_ret_, NULL, NULL)


/**
 * Runs end to end tests for a single input file
 *
 * Some argument's can be ommitted and then the default value will be used
 *
 * @param           i_driver_                 The end to end test driver
 * @param           i_inputs_                 Array of filename/content pair for sources
 * @param           i_expected_ret            The program's expected return value
 * @param[optional] i_stdout_                 The expected stdout of the program in
 *                                            an RFstring or NULL if there should be no stdout
 *                                            or we don't care to check. Default is NULL.
 * @param[optional] i_arguments               The arguments to provide to the compiler as a
 *                                            a cstring. The default is only the filename.
 */
#define ck_end_to_end_run(...)                                      \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_end_to_end_run_with_stdout, 2, __VA_ARGS__)

#endif
