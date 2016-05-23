#include "testsupport_end_to_end.h"

#include <check.h>
#include CLIB_TEST_HELPERS

#include <rfbase/utils/memory.h>
#include <rfbase/string/core.h>
#include <rfbase/string/files.h>
#include <rfbase/string/conversion.h>
#include <rfbase/system/system.h>

#include <compiler_args.h>

struct end_to_end_driver _driver;

struct end_to_end_driver *get_end_to_end_driver()
{
    return &_driver;
}

void setup_end_to_end_tests()
{
    RF_STRUCT_ZERO(&_driver);
    // initialize the compiler instance here (since we need rf_init())
    _driver.compiler = compiler_create(LOG_TARGET_STDOUT, true);
    ck_assert_msg(_driver.compiler, "failed to create the compiler instance");
}

void teardown_end_to_end_tests()
{
    if (_driver.file_names) {
        unsigned i;
        for (i = 0; i < _driver.files_num; ++i) {
            rf_system_delete_file(&_driver.file_names[i]);
            rf_string_deinit(&_driver.file_names[i]);
        }
        free(_driver.file_names);
    }
    compiler_destroy(_driver.compiler);
}

bool end_to_end_driver_create_files(struct end_to_end_driver *d,
                                    struct test_input_pair *inputs,
                                    unsigned num)
{
    FILE *f;
    unsigned i;
    d->files_num = num;

    // create file names
    RF_MALLOC(d->file_names, sizeof(*d->file_names) * num, return false);
    for (i = 0; i < num; ++i) {
        if (!rf_string_copy_in(&d->file_names[i], &inputs[i].filename)) {
            free(d->file_names);
            return false;
        }
    }

    // create files and fill them with the contents
    for (i = 0; i < num; ++i) {
        f = rf_fopen(&d->file_names[i], "wb");
        if (!f) {
            return false;
        }

        if (!rf_string_fwrite(&inputs[i].contents, f, RF_UTF8, RF_ENDIANESS_UNKNOWN)) {
            return false;
        }

        fclose(f);
    }

    return true;
}


bool end_to_end_driver_compile(struct end_to_end_driver *d,
                               const struct test_input_pair *inputs,
                               unsigned inputsn,
                               char *other_args)
{
    static const struct RFstring sep = RF_STRING_STATIC_INIT(" ");
    static const char *exec_name= "refu";
    struct RFstring temp_string;
    uint32_t other_args_number = 0;
    struct RFstring *args_strings = NULL;
    char **args_cstrings;
    unsigned int i;
    bool ret = false;

    if (other_args) {
        if (!rf_string_init(&temp_string, other_args)) {
            return false;
        }

        if (!rf_string_tokenize(&temp_string, &sep, &other_args_number, &args_strings)) {
            // no separating character so only 1 argument
            other_args_number = 1;
        }
    }

    uint32_t args_num = other_args
        ? other_args_number + 1
        : other_args_number + inputsn + 1;
    RF_MALLOC(
        args_cstrings, args_num * sizeof(*args_cstrings),
        goto free_strings_arr
    );
    // executable name is always the first argument
    args_cstrings[0] = (char*)exec_name;


    if (other_args) {
        if (other_args_number == 1) {
            args_cstrings[1] = strdup(other_args);
        } else {
            // unfortunately compiler_pass_args needs normal c strings so we need to null terminate
            for (i = 1; i <= other_args_number; ++i) {
                size_t length = rf_string_length_bytes(&args_strings[i - 1]);
                RF_MALLOC(args_cstrings[i], length + 1, goto free_strings_arr);
                strncpy(args_cstrings[i], args_strings[i - 1].data, length);
                args_cstrings[i][length] = '\0';
            }
        }
    } else {
        // if no arguments were given, add the input file names
        unsigned j;
        for (i = other_args_number + 1, j = 0; j < inputsn; ++i, ++j) {
            size_t length = rf_string_length_bytes(&inputs[j].filename);
            RF_MALLOC(args_cstrings[i], length + 1, goto free_strings_arr);
            strncpy(args_cstrings[i], rf_string_data(&inputs[j].filename), length);
            args_cstrings[i][length] = '\0';
        }
    }

    // + 1 is for the initial argument of the executable name
    if (!compiler_pass_args(args_num, args_cstrings)) {
        goto free_cstrings_arr;
    }

    if (!compiler_process(d->compiler)) {
        // if compile failed show errors to explain why
        compiler_print_errors(d->compiler);
        goto free_cstrings_arr;
    }

    ret = true;
free_cstrings_arr:
    for (i = 1; i < args_num; ++i) {
        free(args_cstrings[i]);
    }
    free(args_cstrings);
free_strings_arr:
    if (args_strings) {
        for (i = 0; i < args_num - 1; i++) {
            rf_string_deinit(&args_strings[i]);
        }
        free(args_strings);
    }

    if (other_args) {
        rf_string_deinit(&temp_string);
    }
    return ret;
}

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value,
                           const struct RFstring *expected_output)
{
    char stdout_buff[1024];
    FILE *proc;
    const struct RFstring* output = compiler_args_get_executable_name(d->compiler->args);

    RFS_PUSH();
    proc = rf_popen(RFS_OR_DIE("./"RFS_PF".exe", RFS_PA(output)), "r");
    RFS_POP();

    if (!proc) {
        return false;
    }

    if (expected_output) {
        size_t output_length = rf_string_length_bytes(expected_output);
        size_t actual_length = fread(stdout_buff, 1, output_length, proc);
        if (output_length != actual_length) {
            ck_abort_msg(
                "Expected \""RFS_PF"\" in stdout but got:\n\"%.*s\"",
                RFS_PA(expected_output),
                actual_length,
                stdout_buff
            );
        }
        ck_assert_rf_str_eq_nntstr(expected_output, stdout_buff, output_length);
    }

    *ret_value = WEXITSTATUS(rf_pclose(proc));
    return true;
}

i_INLINE_INS bool end_to_end_create_files(struct test_input_pair *inputs, unsigned num);
i_INLINE_INS bool end_to_end_compile(const struct test_input_pair *inputs,
                                     unsigned inputsn,
                                     char *other_args);
i_INLINE_INS bool end_to_end_run(int *ret_value,
                                 const struct RFstring *output);
