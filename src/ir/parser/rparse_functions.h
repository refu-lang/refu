#ifndef LFR_IR_PARSE_FUNCTIONS_H
#define LFR_IR_PARSE_FUNCTIONS_H

#include <stdbool.h>

struct rir_parser;
struct token;
struct rir;

bool rir_parse_global(struct rir_parser *p, struct token *tok, struct rir *r);
bool rir_parse_typedef(struct rir_parser *p, struct token *id, bool uniondef, struct rir *r);
#endif
