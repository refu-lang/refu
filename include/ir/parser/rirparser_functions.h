#ifndef LFR_IR_PARSE_FUNCTIONS_H
#define LFR_IR_PARSE_FUNCTIONS_H

#include <stdbool.h>

struct rir_parser;
struct token;
struct rir;
struct rir_type_arr;

struct rir_type *rir_parse_type(struct rir_parser *p, struct rir *r);
bool rir_parse_typearr(struct rir_parser *p, struct rir_type_arr *arr, struct rir *r);
bool rir_parse_global(struct rir_parser *p, struct token *tok, struct rir *r);
bool rir_parse_typedef(struct rir_parser *p, struct token *id, bool uniondef, struct rir *r);
bool rir_parse_fndef(struct rir_parser *p, struct rir *r);
#endif
