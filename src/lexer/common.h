#ifndef LFR_LEXER_COMMON_H
#define LFR_LEXER_COMMON_H
#define lexer_synerr(lexer_, start_, end_, ...)   \
    do {                                          \
        i_info_ctx_add_msg((lexer_)->info,        \
                           MESSAGE_SYNTAX_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
    } while(0)
#endif
