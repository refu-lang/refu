#ifndef LFR_PARSER_RECURSIVE_DESCENT_COMMON_H
#define LFR_PARSER_RECURSIVE_DESCENT_COMMON_H

#define parser_synerr(parser_, location_, ...)    \
    do {                                          \
        i_info_ctx_add_msg((parser_)->info,       \
                           MESSAGE_SYNTAX_ERROR,  \
                           (location_),           \
                           __VA_ARGS__);          \
    } while(0)

#define parser_has_synerr(parser_)                      \
    info_ctx_has((parser_)->info, MESSAGE_SYNTAX_ERROR)

#endif
