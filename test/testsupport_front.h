#ifndef LFR_TESTSUPPORT_FRONTEND_H
#define LFR_TESTSUPPORT_FRONTEND_H
#include <stdbool.h>
#include <check.h>
#include <Data_Structures/darray.h>
#include <inpfile.h>


struct front_testdriver {
    struct inpfile f;
    struct info_ctx *info;
    struct RFstringx buffstr;
    //! A buffer of ast node pointers for quick identifier checks
    //! and easy freeing at test teardown
    struct {darray(struct ast_node*);} nodes;
};

/**
 * gets the global frontend test driver
 * (should be initialized by the setup_front_tests() fixture)
 */
struct front_testdriver *get_front_testdriver();

bool front_testdriver_init(struct front_testdriver *p);
void front_testdriver_deinit(struct front_testdriver *p);

struct inpfile *front_testdriver_get_file(struct front_testdriver *d);

/**
 * Assign a string to the first/only(for now?) file of the driver
 * and return that file
 */
struct inpfile *front_testdriver_assign(struct front_testdriver *d,
                                        const struct RFstring *s);

/**
 * Returns a pointer to the buffer string after having populated it with
 * any parsing errors that may have occured. If no errors occured then
 * returns NULL.
 */
struct RFstringx *front_testdriver_geterrors(struct front_testdriver *d);

/**
 * Generates an ast_identifier with the given string and location
 * and keeps its pointer indexed for freeing at test teardown
 */
struct ast_node *front_testdriver_generate_identifier(
    struct front_testdriver *d,
    struct inplocation *loc,
    const char *s);

void setup_front_tests();
void teardown_front_tests();


#define ck_astcheck_abort(file_, line_, msg_, ...)      \
    ck_abort_msg("Checking ast trees from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

#define ck_assert_ast_node_loc(i_node_, i_sline_, i_scol_, i_eline_, i_ecol_) \
    do {                                                                \
        struct inplocation *loc = &(i_node_)->location;                 \
        ck_assert_uint_eq(loc->start.ine, (i_sline_));                  \
        ck_assert_uint_eq(loc->start.col, (i_scol_));                   \
        ck_assert_uint_eq(loc->end.line, (i_eline_));                   \
        ck_assert_uint_eq(loc->end.col, (i_ecol_));                     \
    } while(0)


#define ck_assert_inpoffset_eq(i_off_, i_bytes_, i_chars_, i_lines_)    \
    do {                                                                \
        ck_assert_uint_eq((i_off_)->bytes_moved, i_bytes_);             \
        ck_assert_uint_eq((i_off_)->chars_moved, i_chars_);             \
        ck_assert_uint_eq((i_off_)->lines_moved, i_lines_);             \
    } while(0)

#define ck_assert_driver_offset_eq(i_driver_, i_bytes_, i_chars_, i_lines_) \
    do {                                                                \
        struct inpoffset *off_ = inputfile_offset(&(i_driver_)->f);     \
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
    } while(0)

#define ck_assert_parser_error(d_, err_)                                \
        do {                                                            \
            static const struct RFstring i_tmps_ = RF_STRING_STATIC_INIT(err_); \
            struct RFstringx *i_tmp_ = parser_testdriver_geterrors(d_); \
            if (!i_tmp_) {                                              \
                ck_abort_msg("Expected parsing error but none found");  \
            }                                                           \
            ck_assert_msg(                                              \
                rf_string_equal(&i_tmps_, i_tmp_),                      \
                "Expected parsing error does not match. Expected:\n"   \
                RF_STR_PF_FMT"\nGot:\n"RF_STR_PF_FMT,                   \
                RF_STR_PF_ARG(&i_tmps_), RF_STR_PF_ARG(i_tmp_));        \
        } while(0)

#define check_ast_match(got_, expect_, inpfile_)            \
        check_ast_match_impl(got_, expect_, inpfile_, __FILE__, __LINE__)

bool check_ast_match_impl(struct ast_node *got,
                          struct ast_node *expect,
                          struct inpfile *ifile,
                          const char* filename,
                          unsigned int line);


#endif
