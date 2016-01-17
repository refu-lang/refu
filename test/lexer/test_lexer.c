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
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "asd { }");
    front = front_testdriver_new_ast_main_source(&s);
    ck_assert_msg(front, "Failed to assign string to file ");
    f = front_testdriver_file();
    struct token expected[] = {
        TESTLEX_IDENTIFIER_INIT(0, 0, 0, 2, "asd"),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 4, 0, 4)
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 6, 0, 6)
        }
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_tokens_2) {
    struct front_ctx *front;
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
        "return\n"
        "^\n"
        ".\n"
        "match\n"
        "=>\n"
        "module\n"
    );
    front = front_testdriver_new_ast_main_source(&s);
    ck_assert_msg(front, "Failed to assign string to file ");
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_TYPE,
            .location=LOC_INIT(f, 0, 0, 0, 3)
        },
        TESTLEX_IDENTIFIER_INIT(0, 5, 0, 7, "foo"),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 9, 0, 9)
        },
        TESTLEX_IDENTIFIER_INIT(0, 11, 0, 11, "a"),
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 12, 0, 12)
        },
        TESTLEX_IDENTIFIER_INIT(0, 13, 0, 15, "i32"),
        {
            .type=TOKEN_OP_COMMA,
            .location=LOC_INIT(f, 0, 16, 0, 16)
        },
        TESTLEX_IDENTIFIER_INIT(0, 18, 0, 18, "b"),
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 19, 0, 19)
        },
        TESTLEX_IDENTIFIER_INIT(0, 20, 0, 25,  "string"),
        {
            .type=TOKEN_OP_TYPESUM,
            .location=LOC_INIT(f, 0, 27, 0, 27)
        },
        TESTLEX_IDENTIFIER_INIT(0, 29, 0, 29, "c"),
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 0, 30, 0, 30)
        },
        TESTLEX_IDENTIFIER_INIT(0, 31, 0, 33, "f32"),
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 35, 0, 35)
        },
        /* 2nd line */
        {
            .type=TOKEN_KW_FUNCTION,
            .location=LOC_INIT(f, 1, 0, 1, 1)
        },
        TESTLEX_IDENTIFIER_INIT(1, 3, 1, 5, "foo"),
        {
            .type=TOKEN_SM_OPAREN,
            .location=LOC_INIT(f, 1, 6, 1, 6)
        },
        TESTLEX_IDENTIFIER_INIT(1, 7, 1, 7, "a"),
        {
            .type=TOKEN_SM_COLON,
            .location=LOC_INIT(f, 1, 8, 1, 8)
        },
        TESTLEX_IDENTIFIER_INIT(1, 9, 1, 11, "int"),
        {
            .type=TOKEN_SM_CPAREN,
            .location=LOC_INIT(f, 1, 12, 1, 12)
        },
        {
            .type=TOKEN_OP_IMPL,
            .location=LOC_INIT(f, 1, 14, 1, 15)
        },
        TESTLEX_IDENTIFIER_INIT(1, 17, 1, 19, "int"),
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
            .type=TOKEN_OP_LOGIC_AND,
            .location=LOC_INIT(f, 12, 0, 12, 1)
        },
        /* 14th line */
        {
            .type=TOKEN_OP_LOGIC_OR,
            .location=LOC_INIT(f, 13, 0, 13, 1)
        },
        /* 15th line */
        {
            .type=TOKEN_KW_RETURN,
            .location=LOC_INIT(f, 14, 0, 14, 5)
        },
        /* 16th line */
        {
            .type=TOKEN_OP_BITWISE_XOR,
            .location=LOC_INIT(f, 15, 0, 15, 0)
        },
        /* 17th line */
        {
            .type=TOKEN_OP_MEMBER_ACCESS,
            .location=LOC_INIT(f, 16, 0, 16, 0)
        },
        /* 18th line */
        {
            .type=TOKEN_KW_MATCH,
            .location=LOC_INIT(f, 17, 0, 17, 4)
        },
        /* 18th line */
        {
            .type=TOKEN_SM_THICKARROW,
            .location=LOC_INIT(f, 18, 0, 18, 1)
        },
        /* 19th line */
        {
            .type=TOKEN_KW_MODULE,
            .location=LOC_INIT(f, 19, 0, 19, 5)
        },
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_tokens_crammed) {
    struct inpfile *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "food<>||");
    front_testdriver_new_ast_main_source(&s);
    f = front_testdriver_file();
    struct token expected[] = {
        TESTLEX_IDENTIFIER_INIT(0, 0, 0, 3, "food"),
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 4, 0, 4)
        },
        {
            .type=TOKEN_OP_GT,
            .location=LOC_INIT(f, 0, 5, 0, 5)
        },
        {
            .type=TOKEN_OP_LOGIC_OR,
            .location=LOC_INIT(f, 0, 6, 0, 7)
        }
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_constant_numbers) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "42\n"
        "3.14\n"
        "0b11100\n"
        "0xFEFfE\n"
        "03452623\n"
        "1.0e-10\n"
        "3.9265E+2\n"
        "0\n"
        "-13\n"
        "-2.1234\n"
    );
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 42),
        TESTLEX_FLOAT_INIT(1, 0, 1, 3, 3.14),
        TESTLEX_INTEGER_INIT(2, 0, 2, 6, 28),
        TESTLEX_INTEGER_INIT(3, 0, 3, 6, 1044478),
        TESTLEX_INTEGER_INIT(4, 0, 4, 7, 939411),
        TESTLEX_FLOAT_INIT(5, 0, 5, 6, 1.0e-10),
        TESTLEX_FLOAT_INIT(6, 0, 6, 8, 3.9265e+2),
        TESTLEX_INTEGER_INIT(7, 0, 7, 0, 0),
        TESTLEX_INTEGER_INIT(8, 0, 8, 2, -13),
        TESTLEX_FLOAT_INIT(9, 0, 9, 6, -2.1234),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_string_literals) {

    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "\"Celka\"\n"
        "\"Containing escaped \\\"\\\" quotes\"\n"
        "\"Eleos そう思いながらも\"\n"
    );
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_LITERAL_INIT(0, 0, 0, 6, 0, 6,  "Celka"),
        TESTLEX_LITERAL_INIT(1, 0, 1, 31, 0, 31,
                             "Containing escaped \\\"\\\" quotes"),
        TESTLEX_LITERAL_INIT(2, 0, 2, 15, 0, 31, "Eleos そう思いながらも")
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST (test_lexer_scan_with_comments) {
    struct front_ctx *front;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "234 // a comment\n"
        "// another comment\n"
        "\"string_literal\"\n"
        "//"
    );
    front = front_testdriver_new_ast_main_source(&s);
    ck_assert_msg(front, "Failed to assign string to file");
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 2, 234),
        TESTLEX_LITERAL_INIT(2, 0, 2, 15, 0, 15, "string_literal")
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

/* -- lexer scan edge cases tests -- */

START_TEST(test_lexer_scan_identifier_at_end) {
    struct inpfile *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT("<Type a bbb");
    front_testdriver_new_ast_main_source(&s);
    f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 0, 0, 0),
        },
        TESTLEX_IDENTIFIER_INIT(0, 1, 0, 4, "Type"),
        TESTLEX_IDENTIFIER_INIT(0, 6, 0, 6, "a"),
        TESTLEX_IDENTIFIER_INIT(0, 8, 0, 10, "bbb"),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_problematic_typeclass) {
    struct inpfile *f;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "class pointers {\n"
        "fn dosth(\n"
        "}");
    front_testdriver_new_ast_main_source(&s);
    f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_TYPECLASS,
            .location=LOC_INIT(f, 0, 0, 0, 4),
        },
        TESTLEX_IDENTIFIER_INIT(0, 6, 0, 13, "pointers"),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 15, 0, 15),
        },
        {
            .type=TOKEN_KW_FUNCTION,
            .location=LOC_INIT(f, 1, 0, 1, 1),
        },
        TESTLEX_IDENTIFIER_INIT(1, 3, 1, 7, "dosth"),
        {
            .type=TOKEN_SM_OPAREN,
            .location=LOC_INIT(f, 1, 8, 1, 8),
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 2, 0, 2, 0),
        },
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_constant_int_at_end) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("13 2462");
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 13),
        TESTLEX_INTEGER_INIT(0, 3, 0, 6, 2462),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_constant_float_at_end) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("13 0.142");
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 13),
        TESTLEX_FLOAT_INIT(0, 3, 0, 7, 0.142),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_string_literal_at_end) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("21 \"Berlin\"");
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 21),
        TESTLEX_LITERAL_INIT(0, 3, 0, 10, 3, 10, "Berlin"),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_integer_with_tokens_in_between) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("10|&{23");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 10),
        {
            .type=TOKEN_OP_TYPESUM,
            .location=LOC_INIT(f, 0, 2, 0, 2),
        },
        {
            .type=TOKEN_OP_AMPERSAND,
            .location=LOC_INIT(f, 0, 3, 0, 3),
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 4, 0, 4),
        },
        TESTLEX_INTEGER_INIT(0, 5, 0, 6, 23),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_float_with_tokens_in_between) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("5.3134|&{23");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        TESTLEX_FLOAT_INIT(0, 0, 0, 5, 5.3134),
        {
            .type=TOKEN_OP_TYPESUM,
            .location=LOC_INIT(f, 0, 6, 0, 6),
        },
        {
            .type=TOKEN_OP_AMPERSAND,
            .location=LOC_INIT(f, 0, 7, 0, 7),
        },
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 8, 0, 8),
        },
        TESTLEX_INTEGER_INIT(0, 9, 0, 10, 23),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_integer_close_to_member_access) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("10).something_else");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 1, 10),
        {
            .type=TOKEN_SM_CPAREN,
            .location=LOC_INIT(f, 0, 2, 0, 2),
        },
        {
            .type=TOKEN_OP_MEMBER_ACCESS,
            .location=LOC_INIT(f, 0, 3, 0, 3),
        },
        TESTLEX_IDENTIFIER_INIT(0, 4, 0, 17, "something_else"),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);
} END_TEST

START_TEST(test_lexer_scan_zero_at_eof) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "0"
    );
    front_testdriver_new_ast_main_source(&s);
    struct token expected[] = {
        TESTLEX_INTEGER_INIT(0, 0, 0, 0, 0),
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_scan_zero_before_curly) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn main() -> u32 {return 0}"
    );
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_FUNCTION,
            .location=LOC_INIT(f, 0, 0, 0, 1),
        },
        TESTLEX_IDENTIFIER_INIT(0, 3, 0, 6, "main"),
        {
            .type=TOKEN_SM_OPAREN,
            .location=LOC_INIT(f, 0, 7, 0, 7),
        },
        {
            .type=TOKEN_SM_CPAREN,
            .location=LOC_INIT(f, 0, 8, 0, 8),
        },
        {
            .type=TOKEN_OP_IMPL,
            .location=LOC_INIT(f, 0, 10, 0, 11),
        },
        TESTLEX_IDENTIFIER_INIT(0, 13, 0, 15, "u32"),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 17, 0, 17),
        },
        {
            .type=TOKEN_KW_RETURN,
            .location=LOC_INIT(f, 0, 18, 0, 23),
        },
        TESTLEX_INTEGER_INIT(0, 25, 0, 25, 0),
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 26, 0, 26),
        },
    };
    ck_assert_lexer_scan("Scanning failed");
    testsupport_lexer_check_tokens(expected);

} END_TEST

START_TEST(test_lexer_push_pop) {
    static const struct RFstring s = RF_STRING_STATIC_INIT("if a < 2 { }");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_IF,
            .location=LOC_INIT(f, 0, 0, 0, 1)
        },
        TESTLEX_IDENTIFIER_INIT(0, 3, 0, 3, "a"),
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 5, 0, 5),
        },
        TESTLEX_INTEGER_INIT(0, 7, 0, 7, 2),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 9, 0, 9),
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 11, 0, 11),
        },
    };
    ck_assert_lexer_scan("Scanning failed");

    // check that simple push/pop does not affect anything
    struct lexer *lex = front_testdriver_lexer();
    struct token *tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[0], tok, 0);
    lexer_push(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[1], tok, 1);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[2], tok, 2);
    lexer_pop(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[3], tok, 3);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[4], tok, 4);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[5], tok, 5);
    tok = lexer_curr_token_advance(lex);
    ck_assert_msg(!tok, "Last token should be NULL");
} END_TEST

START_TEST(test_lexer_push_rollback) {

    static const struct RFstring s = RF_STRING_STATIC_INIT("if a < 2 { }");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_IF,
            .location=LOC_INIT(f, 0, 0, 0, 1)
        },
        TESTLEX_IDENTIFIER_INIT(0, 3, 0, 3, "a"),
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 5, 0, 5),
        },
        TESTLEX_INTEGER_INIT(0, 7, 0, 7, 2),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 9, 0, 9),
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 11, 0, 11),
        },
    };
    ck_assert_lexer_scan("Scanning failed");

    struct lexer *lex = front_testdriver_lexer();
    struct token *tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[0], tok, 0);
    lexer_push(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[1], tok, 1);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[2], tok, 2);
    lexer_rollback(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[1], tok, 1);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[2], tok, 2);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[3], tok, 3);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[4], tok, 4);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[5], tok, 5);
    tok = lexer_curr_token_advance(lex);
    ck_assert_msg(!tok, "Last token should be NULL");
} END_TEST

START_TEST(test_lexer_many_push_rollback) {

    static const struct RFstring s = RF_STRING_STATIC_INIT("if a < 2 { }");
    front_testdriver_new_ast_main_source(&s);
    struct inpfile *f = front_testdriver_file();
    struct token expected[] = {
        {
            .type=TOKEN_KW_IF,
            .location=LOC_INIT(f, 0, 0, 0, 1)
        },
        TESTLEX_IDENTIFIER_INIT(0, 3, 0, 3, "a"),
        {
            .type=TOKEN_OP_LT,
            .location=LOC_INIT(f, 0, 5, 0, 5),
        },
        TESTLEX_INTEGER_INIT(0, 7, 0, 7, 2),
        {
            .type=TOKEN_SM_OCBRACE,
            .location=LOC_INIT(f, 0, 9, 0, 9),
        },
        {
            .type=TOKEN_SM_CCBRACE,
            .location=LOC_INIT(f, 0, 11, 0, 11),
        },
    };
    ck_assert_lexer_scan("Scanning failed");

    struct lexer *lex = front_testdriver_lexer();
    struct token *tok = lexer_curr_token_advance(lex);
    lexer_push(lex);
    ck_assert_tokens_eq(lex, &expected[0], tok, 0);
    lexer_push(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[1], tok, 1);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[2], tok, 2);
    lexer_rollback(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[1], tok, 1);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[2], tok, 2);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[3], tok, 3);
    lexer_push(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[4], tok, 4);
    lexer_rollback(lex);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[4], tok, 4);
    tok = lexer_curr_token_advance(lex);
    ck_assert_tokens_eq(lex, &expected[5], tok, 5);
    tok = lexer_curr_token_advance(lex);
    ck_assert_msg(!tok, "Last token should be NULL");
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
    tcase_add_test(scan, test_lexer_scan_constant_numbers);
    tcase_add_test(scan, test_lexer_scan_string_literals);
    tcase_add_test(scan, test_lexer_scan_with_comments);

    TCase *scan_edge = tcase_create("lexer_scan_edge_cases");
    tcase_add_checked_fixture(scan_edge,
                              setup_front_tests,
                              teardown_front_tests);
    tcase_add_test(scan_edge, test_lexer_scan_identifier_at_end);
    tcase_add_test(scan_edge, test_lexer_scan_problematic_typeclass);
    tcase_add_test(scan_edge, test_lexer_scan_constant_int_at_end);
    tcase_add_test(scan_edge, test_lexer_scan_constant_float_at_end);
    tcase_add_test(scan_edge, test_lexer_scan_string_literal_at_end);
    tcase_add_test(scan_edge, test_lexer_scan_integer_with_tokens_in_between);
    tcase_add_test(scan_edge, test_lexer_scan_float_with_tokens_in_between);
    tcase_add_test(scan_edge, test_lexer_scan_integer_close_to_member_access);
    tcase_add_test(scan_edge, test_lexer_scan_zero_at_eof);
    tcase_add_test(scan_edge, test_lexer_scan_zero_before_curly);

    TCase *lexer_utils = tcase_create("lexer_utilities");
    tcase_add_checked_fixture(lexer_utils,
                              setup_front_tests,
                              teardown_front_tests);
    tcase_add_test(lexer_utils, test_lexer_push_pop);
    tcase_add_test(lexer_utils, test_lexer_push_rollback);
    tcase_add_test(lexer_utils, test_lexer_many_push_rollback);
    

    suite_add_tcase(s, scan);
    suite_add_tcase(s, scan_edge);
    suite_add_tcase(s, lexer_utils);
    return s;
}


