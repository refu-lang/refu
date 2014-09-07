#ifndef LFR_TESTSUPPORT_LEXER_H
#define LFR_TESTSUPPORT_LEXER_H

#include <lexer.h>
#include <stdbool.h>
#include <check.h>

#define ck_lexer_abort(file_, line_, msg_, ...)           \
    ck_abort_msg("Lexer test failed at : %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

#define check_lexer_tokens(l_, tokens_, num_)                           \
    check_lexer_tokens_impl((l_), (tokens_), (num_), __FILE__, __LINE__)

void check_lexer_tokens_impl(struct lexer *l,
                             struct token *tokens,
                             unsigned num,
                             const char *filename,
                             unsigned int line);

#define LOC_TEST_STATIC_INIT(file_, sl_, sc_, el_, ec_, sp_, ep_) \
    .loc = {.file = file_,                                        \
            .start_line = sl_,                                    \
            .start_col = sc_,                                     \
            .end_line = el_,                                      \
            .end_col= ec_,                                        \
            .sp = sp_,                                            \
            .ep = ep_}

#endif
