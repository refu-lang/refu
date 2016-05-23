#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rfbase/string/core.h>
#include <ast/ast.h>

#include "testsupport_front.h"

#include CLIB_TEST_HELPERS

START_TEST(test_acc_ws_simple) {
    struct inpfile *f;
    struct front_ctx *front;
    struct inpoffset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT("  asd    ");
    front = front_testdriver_new_ast_main_source(&s);
    f = front->file;
    ck_assert_msg(f, "Failed to assign string to file ");

    inpfile_acc_ws(f);
    off = inpfile_offset(f);
    ck_assert_inpoffset_eq(off, 2, 2, 0);
    str = inpfile_str(f);
    ck_assert_rf_str_eq_cstr(str, "asd    ");
} END_TEST

START_TEST(test_acc_ws_simple_nl) {
    struct inpfile *f;
    struct front_ctx *front;
    struct inpoffset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        " \n \r \t asd \n   ");
    front = front_testdriver_new_ast_main_source(&s);
    f = front->file;
    ck_assert_msg(f, "Failed to assign string to file ");

    inpfile_acc_ws(f);
    off = inpfile_offset(f);
    ck_assert_inpoffset_eq(off, 7, 7, 1);
    str = inpfile_str(f);
    ck_assert_rf_str_eq_cstr(str, "asd \n   ");
} END_TEST

START_TEST(test_acc_ws_full) {
    struct inpfile *f;
    struct front_ctx *front;
    struct inpoffset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(" \n \r \t \n \n  ");
    front = front_testdriver_new_ast_main_source(&s);
    f = front->file;
    ck_assert_msg(f, "Failed to assign string to file ");

    inpfile_acc_ws(f);
    off = inpfile_offset(f);
    ck_assert_inpoffset_eq(off, 12, 12, 3);
    str = inpfile_str(f);
    ck_assert_rf_str_eq_cstr(str, "");
} END_TEST

START_TEST(test_acc_ws_none) {
    struct inpfile *f;
    struct front_ctx *front;
    struct inpoffset *off;
    struct RFstringx *str;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "identifier_before_space \n \r \t");
    front = front_testdriver_new_ast_main_source(&s);
    f = front->file;
    ck_assert_msg(f, "Failed to assign string to file ");

    inpfile_acc_ws(f);
    off = inpfile_offset(f);
    ck_assert_inpoffset_eq(off, 0, 0, 0);
    str = inpfile_str(f);
    ck_assert_rf_str_eq_cstr(str, "identifier_before_space \n \r \t");
} END_TEST

START_TEST(test_acc_ws_none_empty) {
    struct inpfile *f;
    struct front_ctx *front;
    struct inpoffset *off;
    struct RFstringx *str;
    front = front_testdriver_new_ast_main_source(rf_string_empty_get());
    f = front->file;
    ck_assert_msg(f, "Failed to assign string to file ");

    inpfile_acc_ws(f);
    off = inpfile_offset(f);
    ck_assert_inpoffset_eq(off, 0, 0, 0);
    str = inpfile_str(f);
    ck_assert_rf_str_eq_cstr(str, "");
} END_TEST


Suite *frontend_input_suite_create(void)
{
    Suite *s = suite_create("frontend_input");

    TCase *whitespace = tcase_create("inpfile_acc_whitespace");
    tcase_add_checked_fixture(whitespace,
                              setup_front_tests,
                              teardown_front_tests);
    tcase_add_test(whitespace, test_acc_ws_simple);
    tcase_add_test(whitespace, test_acc_ws_simple_nl);
    tcase_add_test(whitespace, test_acc_ws_full);
    tcase_add_test(whitespace, test_acc_ws_none);
    tcase_add_test(whitespace, test_acc_ws_none_empty);

    suite_add_tcase(s, whitespace);
    return s;
}


