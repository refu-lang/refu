#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <lexer/lexer.h>

#include "../testsupport_front.h"
#include "testsupport_lexer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_lexer_scan_tokens_1) {
    struct front_ctx *front;
    struct inpfile *f;
    struct lexer *lex;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "asd { }");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = &front->file;
    lex = front->lexer;
    struct token expected[] = {
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 0, 0, 2),
            TESTLEX_IDENTIFIER_INIT(d, 0, "asd")
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 4, 0, 4)
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 6, 0, 6)
        }
    };
    ck_assert(lexer_scan(lex));
    check_lexer_tokens(lex, expected);

} END_TEST

START_TEST(test_lexer_scan_tokens_2) {
    struct front_ctx *front;
    struct inpfile *f;
    struct lexer *lex;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo { a:i32, b:string | c:f32 }\n"
        "fn foo(a:int) -> int\n"
        "+\n"
        "-\n"
        "*\n"
        "/\n"
        "==\n"
        "!=\n"
        ">\n"
        ">=\n"
        "<\n"
        "<=\n"
        "&&\n"
        "||\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = &front->file;
    lex = front->lexer;
    struct token expected[] = {
        {
            .type=TOKEN_KW_TYPE,
            .location=LOC_INIT(f, 0, 0, 0, 3)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 5, 0, 7),
            TESTLEX_IDENTIFIER_INIT(d, 0, "foo")
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 9, 0, 9)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 11, 0, 11),
            TESTLEX_IDENTIFIER_INIT(d, 0, "a")
        },
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 12, 0, 12)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 13, 0, 15),
            TESTLEX_IDENTIFIER_INIT(d, 0, "i32")
        },
        {
            .type=TOKEN_OP_COMMA,
            .location=LOC_INIT(f, 0, 16, 0, 16)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 18, 0, 18),
            TESTLEX_IDENTIFIER_INIT(d, 0, "b")
        },
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 19, 0, 19)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 20, 0, 25),
            TESTLEX_IDENTIFIER_INIT(d, 0, "string")
        },
        {
            .type=TOKEN_OP_TYPESUM,
            .location=LOC_INIT(f, 0, 27, 0, 27)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 29, 0, 29),
            TESTLEX_IDENTIFIER_INIT(d, 0, "c")
        },
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 30, 0, 30)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 31, 0, 33),
            TESTLEX_IDENTIFIER_INIT(d, 0, "f32")
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 35, 0, 35)
        },
        /* 2nd line */
        {
            .type=TOKEN_KW_FUNCTION,
            .location=LOC_INIT(f, 1, 0, 1, 1)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 1, 3, 1, 5),
            TESTLEX_IDENTIFIER_INIT(d, 0, "foo")
        },
        {
            .type=TOKEN_SM_OPAREN,
            .location=LOC_INIT(f, 1, 6, 1, 6)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 1, 7, 1, 7),
            TESTLEX_IDENTIFIER_INIT(d, 0, "a")
        },
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 1, 8, 1, 8)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 1, 9, 1, 11),
            TESTLEX_IDENTIFIER_INIT(d, 0, "int")
        },
        {
            .type=TOKEN_SM_CPAREN,
            .location=LOC_INIT(f, 1, 12, 1, 12)
        },
        {
            .type=TOKEN_OP_IMPL,
            .location=LOC_INIT(f, 1, 14, 1, 15)
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 1, 17, 1, 19),
            TESTLEX_IDENTIFIER_INIT(d, 0, "int")
        },
        /* 3rd line */
        {
            .type=TOKEN_OP_PLUS,
            .location=LOC_INIT(f, 2, 0, 2, 0)
        },
        /* 4th line */
        {
            .type=TOKEN_OP_MINUS,
            .location=LOC_INIT(f, 3, 0, 3, 0)
        },
        /* 5th line */
        {
            .type=TOKEN_OP_MULTI,
            .location=LOC_INIT(f, 4, 0, 4, 0)
        },
        /* 6th line */
        {
            .type=TOKEN_OP_DIV,
            .location=LOC_INIT(f, 5, 0, 5, 0)
        },
        /* 7th line */
        {
            .type=TOKEN_OP_EQ,
            .location=LOC_INIT(f, 6, 0, 6, 1)
        },
        /* 8th line */
        {
            .type=TOKEN_OP_NEQ,
            .location=LOC_INIT(f, 7, 0, 7, 1)
        },
        /* 9th line */
        {
            .type=TOKEN_OP_GT,
            .location=LOC_INIT(f, 8, 0, 8, 0)
        },
        /* 10th line */
        {
            .type=TOKEN_OP_GTEQ,
            .location=LOC_INIT(f, 9, 0, 9, 1)
        },
        /* 11th line */
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 10, 0, 10, 0)
        },
        /* 12th line */
        {
            .type=TOKEN_OP_LTEQ,
            .location=LOC_INIT(f, 11, 0, 11, 1)
        },
        /* 13th line */
        {
            .type=TOKEN_OP_LOGICAND,
            .location=LOC_INIT(f, 12, 0, 12, 1)
        },
        /* 14th line */
        {
            .type=TOKEN_OP_LOGICOR,
            .location=LOC_INIT(f, 13, 0, 13, 1)
        },
    };
    ck_assert(lexer_scan(lex));
    check_lexer_tokens(lex, expected);

} END_TEST

START_TEST(test_lexer_scan_tokens_crammed) {
    struct front_ctx *front;
    struct inpfile *f;
    struct lexer *lex;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "food<>||");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = &front->file;
    lex = front->lexer;
    struct token expected[] = {
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 0, 0, 3),
            TESTLEX_IDENTIFIER_INIT(d, 0, "food")
        },
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 4, 0, 4)
        },
        {
            .type=TOKEN_OP_GT,
            .location=LOC_INIT(f, 0, 5, 0, 5)
        },
        {
            .type=TOKEN_OP_LOGICOR,
            .location=LOC_INIT(f, 0, 6, 0, 7)
        }
    };
    ck_assert(lexer_scan(lex));
    check_lexer_tokens(lex, expected);

} END_TEST

START_TEST(test_lexer_scan_identifier_at_end) {
    struct front_ctx *front;
    struct inpfile *f;
    struct lexer *lex;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type a bbb");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = &front->file;
    lex = front->lexer;
    struct token expected[] = {
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 0, 0, 0),
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 1, 0, 4),
            TESTLEX_IDENTIFIER_INIT(d, 0, "Type")
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 6, 0, 6),
            TESTLEX_IDENTIFIER_INIT(d, 0, "a")
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 8, 0, 10),
            TESTLEX_IDENTIFIER_INIT(d, 0, "bbb")
        }
    };
    ck_assert(lexer_scan(lex));
    check_lexer_tokens(lex, expected);

} END_TEST

START_TEST(test_lexer_scan_problematic_typeclass) {
    struct front_ctx *front;
    struct inpfile *f;
    struct lexer *lex;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn dosth(\n"
        "}");
    struct front_testdriver *d = get_front_testdriver();
    front = front_testdriver_assign(d, &s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = &front->file;
    lex = front->lexer;
    struct token expected[] = {
        {
            .type=TOKEN_KW_TYPECLASS,
            .location=LOC_INIT(f, 0, 0, 0, 4),
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 0, 6, 0, 13),
            TESTLEX_IDENTIFIER_INIT(d, 0, "pointers")
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 15, 0, 15),
        },
        {
            .type=TOKEN_KW_FUNCTION,
            .location=LOC_INIT(f, 1, 0, 1, 1),
        },
        {
            .type=TOKEN_IDENTIFIER,
            .location=LOC_INIT(f, 1, 3, 1, 7),
            TESTLEX_IDENTIFIER_INIT(d, 0, "dosth")
        },
        {
            .type=TOKEN_SM_OPAREN,
            .location=LOC_INIT(f, 1, 8, 1, 8),
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 2, 0, 2, 0),
        },
    };
    ck_assert(lexer_scan(lex));
    check_lexer_tokens(lex, expected);

} END_TEST

Suite *lexer_suite_create(void)
{
    Suite *s = suite_create("lexer");

    TCase *scan = tcase_create("lexer_scan");
    tcase_add_checked_fixture(scan,
                              setup_front_tests,
                              teardown_front_tests);
    tcase_add_test(scan, test_lexer_scan_tokens_1);
    tcase_add_test(scan, test_lexer_scan_tokens_2);
    tcase_add_test(scan, test_lexer_scan_tokens_crammed);
    tcase_add_test(scan, test_lexer_scan_identifier_at_end);
    tcase_add_test(scan, test_lexer_scan_problematic_typeclass);

    suite_add_tcase(s, scan);
    return s;
}


