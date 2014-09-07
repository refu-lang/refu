#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <lexer/lexer.h>

#include "../parser/testsupport_parser.h"
#include "testsupport_lexer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_lexer_scan_tokens_1) {
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "asd { }");
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    struct token expected[] = {
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 0, 0, 2),
            {.string = RF_STRING_STATIC_INIT("asd")}
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .loc=LOC_INIT(f, 0, 4, 0, 4)
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .loc=LOC_INIT(f, 0, 6, 0, 6)
        }
    };
    struct lexer lex;
    ck_assert(lexer_init(&lex));
    ck_assert(lexer_scan(&lex, f));
    check_lexer_tokens(&lex, expected, 3);


    lexer_deinit(&lex);
} END_TEST

START_TEST(test_lexer_scan_tokens_2) {
    struct parser_file *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i32, b:string | c:f32 }\n"
        "fn foo(a:int) -> int\n"
        /* "+\n" */
        /* "-\n" */
        /* "*\n" */
        /* "/\n" */
        /* "==\n" */
        /* "!=\n" */
        /* ">\n" */
        /* ">=\n" */
        /* "<\n" */
        /* "<=\n" */
        /* "|\n" */
        /* "&&\n" */
        /* "||\n" */
    );
    struct parser_testdriver *d = get_parser_testdriver();
    f = parser_testdriver_assign(d, &s);
    ck_assert_msg(f, "Failed to assign string to file ");
    struct token expected[] = {
        {
            .type=TOKEN_KW_TYPE,
            .loc=LOC_INIT(f, 0, 0, 0, 3)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 5, 0, 7),
            {.string = RF_STRING_STATIC_INIT("foo")}
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .loc=LOC_INIT(f, 0, 9, 0, 9)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 11, 0, 11),
            {.string = RF_STRING_STATIC_INIT("a")}
        },
        {
            .type=TOKEN_SM_COLON,
            .loc=LOC_INIT(f, 0, 12, 0, 12)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 13, 0, 15),
            {.string = RF_STRING_STATIC_INIT("i32")}
        },
        {
            .type=TOKEN_OP_COMMA,
            .loc=LOC_INIT(f, 0, 16, 0, 16)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 18, 0, 18),
            {.string = RF_STRING_STATIC_INIT("b")}
        },
        {
            .type=TOKEN_SM_COLON,
            .loc=LOC_INIT(f, 0, 19, 0, 19)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 20, 0, 25),
            {.string = RF_STRING_STATIC_INIT("string")}
        },
        {
            .type=TOKEN_OP_TYPESUM,
            .loc=LOC_INIT(f, 0, 27, 0, 27)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 29, 0, 29),
            {.string = RF_STRING_STATIC_INIT("c")}
        },
        {
            .type=TOKEN_SM_COLON,
            .loc=LOC_INIT(f, 0, 30, 0, 30)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 0, 31, 0, 33),
            {.string = RF_STRING_STATIC_INIT("f32")}
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .loc=LOC_INIT(f, 0, 35, 0, 35)
        },
        /* 2nd line */
        {
            .type=TOKEN_KW_FUNCTION,
            .loc=LOC_INIT(f, 1, 0, 1, 1)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 1, 3, 1, 5),
            {.string = RF_STRING_STATIC_INIT("foo")}
        },
        {
            .type=TOKEN_SM_OPAREN,
            .loc=LOC_INIT(f, 1, 6, 1, 6)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 1, 7, 1, 7),
            {.string = RF_STRING_STATIC_INIT("a")}
        },
        {
            .type=TOKEN_SM_COLON,
            .loc=LOC_INIT(f, 1, 8, 1, 8)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 1, 9, 1, 11),
            {.string = RF_STRING_STATIC_INIT("int")}
        },
        {
            .type=TOKEN_SM_CPAREN,
            .loc=LOC_INIT(f, 1, 12, 1, 12)
        },
        {
            .type=TOKEN_OP_IMPL,
            .loc=LOC_INIT(f, 1, 14, 1, 15)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .loc=LOC_INIT(f, 1, 16, 1, 18),
            {.string = RF_STRING_STATIC_INIT("int")}
        },
    };
    struct lexer lex;
    ck_assert(lexer_init(&lex));
    ck_assert(lexer_scan(&lex, f));
    check_lexer_tokens(&lex, expected, sizeof(expected)/sizeof(struct token));

    lexer_deinit(&lex);
} END_TEST


Suite *lexer_suite_create(void)
{
    Suite *s = suite_create("lexer");

    TCase *scan = tcase_create("lexer_scan");
    tcase_add_checked_fixture(scan,
                              setup_parser_tests,
                              teardown_parser_tests);
    tcase_add_test(scan, test_lexer_scan_tokens_1);
    tcase_add_test(scan, test_lexer_scan_tokens_2);

    suite_add_tcase(s, scan);
    return s;
}


