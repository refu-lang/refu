#ifndef LFR_TESTSUPPORT_H
#define LFR_TESTSUPPORT_H

#include <rflib/preprocessor/rf_xmacro_argcount.h>
#include <rflib/string/common.h>
#include <check.h>

void setup_base_tests();
void teardown_base_tests();

/**
 * Abort test at a specific location with a specific message
 */
#define ck_abort_at(...)                                                \
    do {                                                                \
        RFS_PUSH();                                                     \
        RF_SELECT_FUNC_IF_NARGGT(CK_ABORT_AT_WITHVARGS, 4, __VA_ARGS__); \
        RFS_POP();                                                      \
    } while (0)

#define CK_ABORT_LESTR ".\nAt %s:%u\n\t"
#define CK_ABORT_AT_WITHVARGS1(file_, line_, msg_intro_, msg_, ...) \
    ck_abort_msg(                                                   \
        msg_intro_ CK_ABORT_LESTR msg_,                             \
        file_,                                                      \
        line_,                                                      \
        __VA_ARGS__                                                 \
    )

#define CK_ABORT_AT_WITHVARGS0(file_, line_, msg_intro_, msg_)  \
    ck_abort_msg(                                               \
        msg_intro_ CK_ABORT_LESTR msg_,                         \
        file_,                                                  \
        line_                                                   \
    )

// make a copy of a static array and put the copy in the darray.
// We are doing this only if the darray takes ownership of the copy and will
// free it later. If we just did a raw copy, there would be an attempt to free
// a static array.
#define testsupport_arr_to_darray(darr_, arr_, type_)       \
    do {                                                    \
        type_ *newarr = malloc(sizeof(arr_));               \
        if (!newarr) {                                      \
            ck_abort_msg("Failed to allocate an array");    \
        }                                                   \
        memcpy(newarr, arr_, sizeof(arr_));                 \
        darray_raw_copy(                                    \
            darr_,                                          \
            newarr,                                         \
            sizeof(arr_) / sizeof(type_)                    \
        );                                                  \
    } while (0)

#endif
