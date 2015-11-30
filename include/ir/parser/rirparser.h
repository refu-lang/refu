#ifndef LFR_IR_RIR_PARSER_H
#define LFR_IR_RIR_PARSER_H

#include <String/rf_str_core.h>
#include <lexer/lexer.h>

struct inpfile;
struct rir;

struct rir_parser {
    //! The buffer for the parsed string
    struct RFstringx buff;
    //! The input file representation
    struct inpfile *file;
    //! The lexer part of the parser
    struct lexer lexer;
    //! The parsed rir, which is the output of parsing
    struct rir *rir;
};

struct rir_parser *rir_parser_create();
bool rir_parser_init(struct rir_parser *p);
void rir_parser_destroy(struct rir_parser *p);
void rir_parser_deinit(struct rir_parser *p);


bool rir_parser_parse_file(struct rir_parser *p, const struct RFstring *name);
bool rir_parser_parse_string(struct rir_parser *p, const struct RFstring *name, struct RFstring *str);

#endif
