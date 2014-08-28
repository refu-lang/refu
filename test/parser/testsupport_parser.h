#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H
#include <stdbool.h>
#include <check.h>

#include <parser/file.h>

struct parser_file;
struct RFstring;

struct parser_testdriver {
    struct parser_file f;
};

/**
 * gets the global parser test driver
 * (should be initialized by the setup_parser_tests() fixture)
 */
struct parser_testdriver *get_parser_testdriver();

bool parser_testdriver_init(struct parser_testdriver *p);
void parser_testdriver_deinit(struct parser_testdriver *p);

struct parser_file *parser_testdriver_get_file(struct parser_testdriver *d);

/**
 * Assign a string to the first/only(for now?) file of the driver
 * and return that file
 */
struct parser_file *parser_testdriver_assign(struct parser_testdriver *d,
                                             const struct RFstring *s);


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


bool check_ast_match(struct ast_node *got, struct ast_node *expect);


#endif
