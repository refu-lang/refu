#ifndef LFR_TESTSUPPORT_LEXER_H
#define LFR_TESTSUPPORT_LEXER_H

#include <lexer/lexer.h>
#include <stdbool.h>
#include <check.h>

#define TESTLEX_IDENTIFIER_INIT(driver_, loc_, str_)                 \
    {.identifier.id =                                                \
        front_testdriver_generate_identifier(driver_, loc_, str_)    \
        }


#define ck_lexer_abort(file_, line_, msg_, ...)           \
    ck_abort_msg("Lexer test failed at : %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

#define check_lexer_tokens(l_, tokens_)                           \
    check_lexer_tokens_impl((l_), (tokens_),                      \
                            sizeof(tokens_)/sizeof(struct token), \
                            __FILE__, __LINE__)

void check_lexer_tokens_impl(struct lexer *l,
                             struct token *tokens,
                             unsigned num,
                             const char *filename,
                             unsigned int line);


#endif
