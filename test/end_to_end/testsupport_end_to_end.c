#include "testsupport_end_to_end.h"

#include <check.h>
#include CLIB_TEST_HELPERS

#include <Utils/memory.h>
#include <String/rf_str_core.h>
#include <String/rf_str_files.h>
#include <String/rf_str_conversion.h>
#include <System/rf_system.h>

#include <compiler_args.h>

struct end_to_end_driver _driver;

struct end_to_end_driver *get_end_to_end_driver()
{
    return &_driver;
}

void setup_end_to_end_tests()
{
    RF_STRUCT_ZERO(&_driver);
    ck_assert_msg(compiler_init(&_driver.compiler),
                  "Failed to initialize compiler in end to end tests setup");
}

void teardown_end_to_end_tests()
{
    if (_driver.file_name) {
        rf_system_delete_file(_driver.file_name);
        rf_string_destroy(_driver.file_name);
    }
    compiler_deinit(&_driver.compiler);
}

bool end_to_end_driver_create_file(struct end_to_end_driver *d,
                                   const char *file_name,
                                   const struct RFstring *s)
{
    FILE *f;
    d->file_name = rf_string_create(file_name);
    if (!d->file_name) {
        return false;
    }

    f = fopen(file_name, "wb");
    if (!f) {
        return false;
    }

    if (!rf_string_fwrite(s, f, RF_UTF8, RF_ENDIANESS_UNKNOWN)) {
        return false;
    }

    fclose(f);

    return true;
}


bool end_to_end_driver_compile(struct end_to_end_driver *d, char *args)
{
    static const struct RFstring sep = RF_STRING_STATIC_INIT(" ");
    static const char *exec_name= "refu";
    struct RFstring temp_string;
    uint32_t args_number;
    struct RFstring *args_strings = NULL;
    char **args_cstrings;
    unsigned int i;
    bool ret = false;
    if (!rf_string_init(&temp_string, args)) {
        return false;
    }

    if (!rf_string_tokenize(&temp_string, &sep, &args_number, &args_strings)) {
        // no separating character so only 1 argument
        args_number = 1;
    }

    RF_MALLOC(args_cstrings, (args_number + 1) * sizeof(*args_cstrings),
              goto free_strings_arr);

    // executable name is always the first argument
    args_cstrings[0] = (char*)exec_name;
    if (args_number == 1) {
        args_cstrings[1] = args;
    } else {
        // unfortunately compiler_pass_args needs normal c strings so we need to null terminate
        for (i = 1; i <= args_number; i++) {
            size_t length = rf_string_length_bytes(&args_strings[i - 1]);
            RF_MALLOC(args_cstrings[i], length + 1, goto free_strings_arr);
            strncpy(args_cstrings[i], args_strings[i - 1].data, length);
            args_cstrings[i][length] = '\0';
        }
    }

    // + 1 is for the initial argument of the executable name
    if (!compiler_pass_args(&d->compiler, args_number + 1, args_cstrings)) {
        goto free_cstrings_arr;
    }

    if (!compiler_process(&d->compiler)) {
        goto free_cstrings_arr;
    }

    ret = true;
free_cstrings_arr:
    if (args_strings) {
        for (i = 1; i <= args_number; ++i) {
            free(args_cstrings[i]);
        }
    }
    free(args_cstrings);
free_strings_arr:
    if (args_strings) {
        for (i = 0; i < args_number; i++) {
            rf_string_deinit(&args_strings[i]);
        }
        free(args_strings);
    }

    rf_string_deinit(&temp_string);
    return ret;
}

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value,
                           const struct RFstring *expected_output)
{
    char stdout_buff[1024];
    FILE *proc;
    struct RFstring *s;
    const struct RFstring* output = compiler_args_get_output(d->compiler.args);

    RFS_PUSH();
    proc = rf_popen(RFS_OR_DIE("./"RF_STR_PF_FMT".exe", RF_STR_PF_ARG(output)), "r");
    RFS_POP();

    if (!proc) {
        return false;
    }

    if (expected_output) {
        size_t output_length = rf_string_length_bytes(expected_output);
        size_t actual_length = fread(stdout_buff, 1, output_length, proc);
        if (output_length != actual_length) {
            ck_abort_msg("Expected \""RF_STR_PF_FMT"\" in stdout but got:\n\"%.*s\"",
                         RF_STR_PF_ARG(expected_output), actual_length, stdout_buff);
        }
        ck_assert_rf_str_eq_nntstr(expected_output, stdout_buff, output_length);
    }


    *ret_value = WEXITSTATUS(rf_pclose(proc));
    return true;
}
