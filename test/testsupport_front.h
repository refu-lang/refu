#ifndef LFR_TESTSUPPORT_FRONTEND_H
#define LFR_TESTSUPPORT_FRONTEND_H

#include <stdbool.h>
#include <check.h>
#include <Data_Structures/darray.h>
#include <inpfile.h>
#include <front_ctx.h>
#include <ast/ast.h>


struct front_testdriver {
    struct front_ctx front;
    struct RFstringx buffstr;
    //! A buffer of ast node pointers for easy freeing
    //! of some nodes at test teardown
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
 * Assign a string to the  file of the driver
 * and return the frontend context.
 */
struct front_ctx *front_testdriver_assign(struct front_testdriver *d,
                                          const struct RFstring *s);

/**
 * Returns a pointer to the buffer string after having populated it with
 * any parsing errors that may have occured. If no errors occured then
 * returns NULL.
 */
struct RFstringx *front_testdriver_geterrors(struct front_testdriver *d);

/**
 * Generate an ast_nodes with the given type, location and value
 * and keep their pointer indexed for freeing at test teardown
 */
struct ast_node *front_testdriver_generate_identifier(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    const char *s);

struct ast_node *front_testdriver_generate_string_literal(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    unsigned int sl_byte_off, unsigned int el_byte_off,
    const char *s);

struct ast_node *front_testdriver_generate_constant_float(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    double val);

struct ast_node *front_testdriver_generate_constant_integer(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    uint64_t val);

struct ast_node *do_front_testdriver_generate_node(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    enum ast_type type, unsigned int args_num, ...);

#define front_testdriver_generate_node(identifier_, ...)                \
    struct ast_node *identifier_;                                       \
    do {                                                                \
        identifier_ = do_front_testdriver_generate_node(__VA_ARGS__);   \
    } while(0)

void setup_front_tests();
void setup_front_tests_with_file_log();
void teardown_front_tests();


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
        struct inpoffset *off_ = inputfile_offset((i_driver_)->f);     \
        ck_assert_uint_eq(off_->bytes_moved, i_bytes_);                 \
        ck_assert_uint_eq(off_->chars_moved, i_chars_);                 \
        ck_assert_uint_eq(off_->lines_moved, i_lines_);                 \
    } while(0)


#define check_ast_match(got_, expect_, inpfile_)            \
        check_ast_match_impl(got_, expect_, inpfile_, __FILE__, __LINE__)

bool check_ast_match_impl(struct ast_node *got,
                          struct ast_node *expect,
                          struct inpfile *ifile,
                          const char* filename,
                          unsigned int line);

#define TESTSUPPORT_INFOMSG_INIT_START(file_, type_, msg_, sl_, sc_)    \
    {                                                                   \
        .s = RF_STRING_STATIC_INIT(msg_),                               \
            .type = type_,                                              \
            .start_mark = LOCMARK_INIT(file_, sl_, sc_)                 \
    }


#define TESTSUPPORT_INFOMSG_INIT_BOTH(file_, type_, msg_, sl_, sc_, el_, ec_) \
    {                                                                   \
        .s = RF_STRING_STATIC_INIT(msg_),                               \
            .type = type_,                                              \
            .start_mark = LOCMARK_INIT(file_, sl_, sc_),                \
            .end_mark = LOCMARK_INIT(file_, el_, ec_)                   \
    }

#endif
