#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H
#include <stdbool.h>

struct parser_file;
struct RFstring;
bool parser_file_dummy_assign(struct parser_file *f, const struct RFstring *s);

void setup_parser_tests();
void teardown_parser_tests();

#endif
