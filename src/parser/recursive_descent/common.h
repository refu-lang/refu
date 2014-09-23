#ifndef LFR_PARSER_RECURSIVE_DESCENT_COMMON_H
#define LFR_PARSER_RECURSIVE_DESCENT_COMMON_H

#include <parser/parser.h>
// TODO: Change both this and the lexer macro to something better
#define parser_synerr(parser_, start_, end_, ...) \
    do {                                          \
        i_info_ctx_add_msg((parser_)->info,       \
                           MESSAGE_SYNTAX_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
        parser_set_syntax_error(parser_);         \
    } while(0)

#endif
