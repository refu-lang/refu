#ifndef LFR_TESTSUPPORT_FRONTEND_H
#define LFR_TESTSUPPORT_FRONTEND_H

#include <stdbool.h>
#include <check.h>
#include <Data_Structures/darray.h>
#include <inpfile.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <compiler.h>


struct front_testdriver {
    struct compiler *compiler;
    struct front_ctx *current_front;
    struct module *current_module;
    //! A buffer of ast node pointers for easy freeing
    //! of some nodes at test teardown
    struct {darray(struct ast_node*);} nodes;
};

/**
 * gets the global frontend test driver
 * (should be initialized by the setup_front_tests() fixture)
 */
struct front_testdriver *get_front_testdriver();

bool front_testdriver_init(struct front_testdriver *p, bool with_stdlib, int rf_logtype);
void front_testdriver_deinit(struct front_testdriver *p);

/**
 * Get the current front_ctx of the testdriver
 */
struct front_ctx *front_testdriver_curr_front();

/**
 * Set the index of the current front_ctx of the testdriveer
 */
void front_testdriver_set_curr_front(unsigned i);

/**
 * Set the index of the current module being tested
 */
void front_testdriver_set_curr_module(unsigned i);

/**
 * Get the first module being tested
 */
struct module *front_testdriver_module();
struct ast_node *front_testdriver_module_root();
/**
 * Get the parser of the current front_ctx being tested
 */
struct parser *front_testdriver_parser();
/**
 * Get the lexer of the current front_ctx being tested
 */
struct lexer *front_testdriver_lexer();
/**
 * Get the ast root of the current front_ctx
 */
struct ast_node *front_testdriver_root();
/**
 * Get the inpfile of the current front_ctx being tested
 */
struct inpfile *front_testdriver_file();
/**
 * Get the inpfile of a specific front_ctx being tested
 */
struct inpfile *front_testdriver_specific_file(unsigned i);

void front_testdriver_create_analyze_stdlib(struct front_testdriver *d);

/**
 * Create a new front_ctx with the given source string containing the main entry to a program
 */
struct front_ctx *front_testdriver_new_main_source(const struct RFstring *s);

/**
 * Create a new front_ctx with the given source
 */
struct front_ctx *front_testdriver_new_source(const struct RFstring *s);

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
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    const char *s);

struct ast_node *front_testdriver_generate_string_literal(
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    unsigned int sl_byte_off, unsigned int el_byte_off,
    const char *s);

struct ast_node *front_testdriver_generate_constant_float(
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    double val);

struct ast_node *front_testdriver_generate_constant_integer(
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    uint64_t val);

struct ast_node *do_front_testdriver_generate_node(
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    enum ast_type type, unsigned int args_num, ...);

#define front_testdriver_generate_node(identifier_, ...)                \
    struct ast_node *identifier_;                                       \
    do {                                                                \
        identifier_ = do_front_testdriver_generate_node(__VA_ARGS__);   \
    } while(0)

void setup_front_tests();
void setup_front_tests_no_stdlib();
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

#define TESTSUPPORT_INFOMSG_INIT_START(type_, msg_, sl_, sc_)           \
    {                                                                   \
        .s = RF_STRING_STATIC_INIT(msg_),                               \
            .type = type_,                                              \
            .start_mark = LOCMARK_INIT(get_front_testdriver()->current_front->file, sl_, sc_) \
            }


#define TESTSUPPORT_INFOMSG_INIT_BOTH(type_, msg_, sl_, sc_, el_, ec_)  \
    {                                                                   \
        .s = RF_STRING_STATIC_INIT(msg_),                               \
            .type = type_,                                              \
            .start_mark = LOCMARK_INIT(get_front_testdriver()->current_front->file, sl_, sc_), \
            .end_mark = LOCMARK_INIT(get_front_testdriver()->current_front->file, el_, ec_) \
            }

#define TESTSUPPORT_INFOMSG_INIT_BOTH_SPECIFIC_FRONT(findex_, type_, msg_, sl_, sc_, el_, ec_) \
    {                                                                   \
        .s = RF_STRING_STATIC_INIT(msg_),                               \
            .type = type_,                                              \
            .start_mark = LOCMARK_INIT(front_testdriver_specific_file(findex_), sl_, sc_), \
            .end_mark = LOCMARK_INIT(front_testdriver_specific_file(findex_), el_, ec_) \
            }

#endif
