#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H
#include <stdbool.h>
#include <check.h>

#include <parser/file.h>


struct parser_testdriver {
    struct parser_file f;
    struct RFstringx buffstr;
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

/**
 * Returns a pointer to the buffer string after having populated it with
 * any parsing errors that may have occured. If no errors occured then
 * returns NULL.
 */
struct RFstringx *parser_testdriver_geterrors(struct parser_testdriver *d);

void setup_parser_tests();
void teardown_parser_tests();


#define ck_astcheck_abort(file_, line_, msg_, ...)      \
    ck_abort_msg("Checking ast trees from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

#define ck_assert_ast_node_loc(i_node_, i_sline_, i_scol_, i_eline_, i_ecol_) \
    do {                                                                \
        struct ast_location *loc = &(i_node_)->location;                \
        ck_assert_uint_eq(loc->start_line, (i_sline_));                 \
        ck_assert_uint_eq(loc->start_col, (i_scol_));                   \
        ck_assert_uint_eq(loc->end_line, (i_eline_));                   \
        ck_assert_uint_eq(loc->end_col, (i_ecol_));                     \
    } while(0)


#define ck_assert_parser_offset_eq(i_off_, i_bytes_, i_chars_, i_lines_) \
    do {                                                                \
        ck_assert_uint_eq((i_off_)->bytes_moved, i_bytes_);             \
        ck_assert_uint_eq((i_off_)->chars_moved, i_chars_);             \
        ck_assert_uint_eq((i_off_)->lines_moved, i_lines_);             \
    } while(0)

#define ck_assert_driver_offset_eq(i_driver_, i_bytes_, i_chars_, i_lines_) \
    do {                                                                \
        struct parser_offset *off_ = parser_file_offset(&(i_driver_)->f); \
        ck_assert_uint_eq(off_->bytes_moved, i_bytes_);                 \
        ck_assert_uint_eq(off_->chars_moved, i_chars_);                 \
        ck_assert_uint_eq(off_->lines_moved, i_lines_);                 \
    } while(0)


#define ck_assert_parsed_node(n_, d_, msg_)                             \
    do {                                                                \
        if (!(n_)) {                                                    \
            struct RFstringx *tmp_ = parser_testdriver_geterrors(d_);   \
            if (tmp_) {                                                 \
                ck_abort_msg(msg_" -- with parser errors\n"RF_STR_PF_FMT, \
                             RF_STR_PF_ARG(tmp_));                      \
            } else {                                                    \
                ck_abort_msg(msg_" -- with no parser errors");          \
            }                                                           \
        }                                                               \
    } while (0)

#define check_ast_match(got_, expect_)                      \
    check_ast_match_impl(got_, expect_, __FILE__, __LINE__)

bool check_ast_match_impl(struct ast_node *got,
                          struct ast_node *expect,
                          const char* filename,
                          unsigned int line);


#endif
