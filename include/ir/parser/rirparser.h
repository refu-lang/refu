#ifndef LFR_IR_RIR_PARSER_H
#define LFR_IR_RIR_PARSER_H

#include <String/rf_str_core.h>
#include <lexer/lexer.h>

struct inpfile;
struct rir;
struct info_ctx;

struct rir_parser {
    //! The buffer for the parsed string
    struct RFstringx buff;
    //! The input file representation
    struct inpfile *file;
    //! The lexer part of the parser
    struct lexer lexer;
    //! Pointer to the common info context
    struct info_ctx *info;
    //! The parsed rir, which is the output of parsing
    struct rir *rir;
    //! The current function during parsing
    struct rir_fndef *curr_fn;
};

struct rir_parser *rir_parser_create(const struct RFstring *name,
                                     const struct RFstring *contents);
bool rir_parser_init(struct rir_parser *p,
                     const struct RFstring *name,
                     const struct RFstring *contents);
void rir_parser_destroy(struct rir_parser *p);
void rir_parser_deinit(struct rir_parser *p);


bool rir_parse(struct rir_parser *p);

#define rirparser_synerr(parser_, start_, end_, ...) \
    do {                                          \
        i_info_ctx_add_msg((parser_)->info,       \
                           MESSAGE_SYNTAX_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
    } while(0)

#endif
