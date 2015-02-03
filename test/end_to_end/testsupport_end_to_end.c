#include "testsupport_end_to_end.h"

#include <check.h>

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
        for (i = 1; i <= args_number; i++) {
            args_cstrings[i] = args_strings[i].data;
        }
    }
    args_number += 1;

    if (!compiler_pass_args(&d->compiler, args_number, args_cstrings)) {
        goto free_cstrings_arr;
    }

    if (!compiler_process(&d->compiler)) {
        goto free_cstrings_arr;
    }

    ret = true;
free_cstrings_arr:
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

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value)
{
    const struct RFstring* output = compiler_args_get_output(d->compiler.args);
    FILE *proc;

    RFS_buffer_push();
    proc = rf_popen(RFS_("./"RF_STR_PF_FMT".exe", RF_STR_PF_ARG(output)), "r");
    RFS_buffer_pop();
    
    if (!proc) {
        return false;
    }

    *ret_value = WEXITSTATUS(rf_pclose(proc));
    return true;
}
