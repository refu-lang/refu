#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H
#include <stdbool.h>
#include <check.h>

struct parser_file;
struct RFstring;

struct parser_file *parser_file_dummy_get();
bool parser_file_dummy_assign(struct parser_file *f, const struct RFstring *s);


void setup_parser_tests();
void teardown_parser_tests();



#define ck_assert_ast_node_loc(i_node_, i_sline_, i_scol_, i_eline_, i_ecol_) \
    do {                                                                \
        struct ast_location *loc = &(i_node_)->location;                \
        ck_assert_uint_eq(loc->start_line, (i_sline_));                 \
        ck_assert_uint_eq(loc->start_col, (i_scol_));                   \
        ck_assert_uint_eq(loc->end_line, (i_eline_));                   \
        ck_assert_uint_eq(loc->end_col, (i_ecol_));                     \
    } while(0)



#endif
