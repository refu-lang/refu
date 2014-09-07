#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <lexer.h>

#include "../parser/testsupport_parser.h"
#include "testsupport_lexer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_lexer_tokens_1) {
    struct parser_file *f;
    char *p;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "asd { }");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    p = parser_file_sp(f);
    ck_assert_msg(f, "Failed to assign string to file ");
    struct token expected[] = {
        {
            .type=TOKEN_IDENTIFIER,
            LOC_TEST_STATIC_INIT(f, 0, 0, 0, 2, p, p + 2),
            {.string = RF_STRING_STATIC_INIT("asd")}
        },
        {
            .type=TOKEN_SM_OCBRACE,
            LOC_TEST_STATIC_INIT(f, 0, 4, 0, 4, p + 4, p + 4)
        },
        {
            .type=TOKEN_SM_CCBRACE,
            LOC_TEST_STATIC_INIT(f, 0, 6, 0, 6, p + 6, p + 6)
        }
    };
    struct lexer lex;
    ck_assert(lexer_init(&lex));
    ck_assert(lexer_scan(&lex, f));
    check_lexer_tokens(&lex, expected, 3);


    lexer_deinit(&lex);
} END_TEST



Suite *lexer_suite_create(void)
{
    Suite *s = suite_create("lexer");

    TCase *basic = tcase_create("lexer_basic");
    tcase_add_checked_fixture(basic,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(basic, test_lexer_tokens_1);

    suite_add_tcase(s, basic);
    return s;
}


