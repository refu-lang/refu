#ifndef LFR_TESTSUPPORT_LEXER_H
#define LFR_TESTSUPPORT_LEXER_H

#include <lexer/lexer.h>
#include <stdbool.h>
#include <check.h>

#define TESTLEX_IDENTIFIER_INIT(sl_, sc_, el_, ec_,  val_)              \
    {                                                                   \
        .type=TOKEN_IDENTIFIER,                                         \
        .location=LOC_INIT(front_testdriver_file(), sl_, sc_, el_, ec_), \
        .value.v=                                                       \
        front_testdriver_generate_identifier(sl_, sc_, el_, ec_, val_)  \
    }

#define TESTLEX_INTEGER_INIT(sl_, sc_, el_, ec_,  val_)                 \
    {                                                                   \
        .type=TOKEN_CONSTANT_INTEGER,                                   \
        .location=LOC_INIT(front_testdriver_file(), sl_, sc_, el_, ec_), \
        .value.v=                                                       \
        front_testdriver_generate_constant_integer(sl_, sc_, el_, ec_, val_) \
    }

#define TESTLEX_FLOAT_INIT(sl_, sc_, el_, ec_,  val_)                   \
    {                                                                   \
        .type=TOKEN_CONSTANT_FLOAT,                                     \
        .location=LOC_INIT(front_testdriver_file(), sl_, sc_, el_, ec_), \
        .value.v=                                                       \
        front_testdriver_generate_constant_float(sl_, sc_, el_, ec_, val_) \
    }

/**
 * Utility macro to initialize string literal token for testing
 * @param sl_         The starting line of the token
 * @param sc_         The starting column of the token
 * @param el_         The ending line of the token
 * @param ec_         The ending column of the token
 * @param sp_         Number of bytes from the start of sl_ till
 *                    the start of the token
 * @param ep_         Number of bytes from the start of el_ till
 *                    the start of the token
 * @param val_        Token's expected value
 */
#define TESTLEX_LITERAL_INIT(sl_, sc_, el_, ec_, sp_, ep_, val_)        \
    {                                                                   \
        .type=TOKEN_STRING_LITERAL,                                     \
        .location=LOC_INIT_FULL(                                        \
            sl_, sc_, el_, ec_,                                         \
            inpfile_line_p(front_testdriver_file(), sl_) + sp_,         \
            inpfile_line_p(front_testdriver_file(), el_) + ep_),        \
        .value.v=                                                       \
        front_testdriver_generate_string_literal(sl_, sc_, el_, ec_, sp_, ep_, val_) \
    }

#define ck_lexer_abort(file_, line_, msg_, ...)           \
    ck_abort_msg("Lexer test failed at : %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

#define testsupport_lexer_check_tokens( tokens_)                    \
    check_lexer_tokens_impl(front_testdriver_lexer(), (tokens_),    \
                            sizeof(tokens_)/sizeof(struct token),   \
                            __FILE__, __LINE__)

void check_lexer_tokens_impl(struct lexer *l,
                             struct token *tokens,
                             unsigned num,
                             const char *filename,
                             unsigned int line);

#define ck_assert_lexer_scan(msg_)                                      \
    do {                                                                \
        if (!lexer_scan(front_testdriver_lexer())) {                    \
            struct RFstringx *tmp_ = front_testdriver_geterrors(get_front_testdriver()); \
            if (tmp_) {                                                 \
                ck_abort_msg(msg_" -- with scanning errors\n"RF_STR_PF_FMT, \
                             RF_STR_PF_ARG(tmp_));                      \
            } else {                                                    \
                ck_abort_msg(msg_" -- with no scanning errors");        \
            }                                                           \
        }                                                               \
    } while(0)

bool test_tokens_cmp(struct token *expected,
                     struct token *got,
                     unsigned int index,
                     struct inpfile *f,
                     const char *filename,
                     unsigned int line);

#define ck_assert_tokens_eq(lexer_, expected_, got_, index_)            \
    test_tokens_cmp((expected_), (got_), index_, (lexer_)->file, __FILE__, __LINE__)

#endif
