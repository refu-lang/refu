#ifndef LFR_TESTSUPPORT_ANALYZER_H
#define LFR_TESTSUPPORT_ANALYZER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <inplocation.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include <analyzer/types.h>
#include <analyzer/typecheck.h>



struct ast_node;

struct analyzer_testdriver {
    //! A buffer of types for quick type checks
    //! and easy freeing at test teardown
    struct {darray(struct type*);} types;
};
struct analyzer_testdriver *get_analyzer_testdriver();

void setup_analyzer_tests();
void teardown_analyzer_tests();

#define testsupport_analyzer_prepare(driver_, msg_)                     \
    do {                                                                \
        ck_assert(lexer_scan((driver_)->front.lexer));                  \
        if (!(parser_process_file((driver_)->front.parser))) {          \
            struct RFstringx *tmp_ = front_testdriver_geterrors(driver_); \
            if (tmp_) {                                                 \
                ck_abort_msg(msg_" -- at parsing with errors:\n"RF_STR_PF_FMT, \
                             RF_STR_PF_ARG(tmp_));                      \
            } else {                                                    \
                ck_abort_msg(msg_" -- at parsing with no parser errors"); \
            }                                                           \
        }                                                               \
        (driver_)->front.analyzer->root = parser_yield_ast_root((driver_)->front.parser); \
    } while(0)

#define testsupport_symbol_table_add_node(st_, driver_, id_, node_)     \
    do {                                                                \
        ck_assert(symbol_table_add_node(st_, (driver_)->front.analyzer, id_, node_)); \
    } while(0)

#define testsupport_symbol_table_lookup_node(st_, idstring_, retval_, first_st) \
    do {                                                                \
        bool at_first_st;                                               \
        retval_ = symbol_table_lookup_node(st_, idstring_, &at_first_st); \
        ck_assert_msg(retval_, "Symbol table lookup failed");           \
        ck_assert_msg(first_st == at_first_st,                          \
                      "symbol found but not, in the expected table");   \
    } while(0)

#define testsupport_symbol_table_lookup_record(st_, idstring_, retval_, first_st) \
    do {                                                                \
        bool at_first_st;                                               \
        retval_ = symbol_table_lookup_record(st_, idstring_, &at_first_st); \
        ck_assert_msg(retval_, "Symbol table lookup failed");           \
        ck_assert_msg(first_st == at_first_st,                          \
                      "symbol found but not, in the expected table");   \
    } while(0)

#define testsupport_types_equal(t1_, t2_)                               \
    do {                                                                \
        ck_assert_msg(type_equals(t1_, t2_, NULL), "expected type mismatch"); \
    } while(0)


struct type *testsupport_analyzer_type_create_builtin(enum builtin_type btype);

struct type *testsupport_analyzer_type_create_operator(enum typeop_type type,
                                                       struct type *left,
                                                       struct type *right);

struct type *testsupport_analyzer_type_create_leaf(const struct RFstring *id,
                                                   struct type *type);

struct type *testsupport_analyzer_type_create_defined(const struct RFstring *id,
                                                      struct type_composite *t);

struct type *testsupport_analyzer_type_create_function(struct type *arg,
                                                       struct type *ret);


/* -- typecheck related support -- */


#define testsupport_typecheck_prepare(driver_)                          \
    do {                                                                \
        testsupport_analyzer_prepare(driver_,                           \
                                     "Preparing for the analyzer phase failed"); \
        ck_assert_msg(analyzer_create_symbol_tables((driver_)->front.analyzer), \
                      "Creating symbol tables failed");                 \
    } while (0)


#define ck_assert_typecheck_ok(d_)                                      \
    do {                                                                \
        if (!analyzer_typecheck((d_)->front.analyzer)) {                \
            struct RFstringx *tmp_ = front_testdriver_geterrors(d_);    \
            if (tmp_) {                                                 \
                ck_abort_msg("Typecheck failed -- with analyzer "       \
                             "errors\n"RF_STR_PF_FMT,                   \
                             RF_STR_PF_ARG(tmp_));                      \
            } else {                                                    \
                ck_abort_msg("Typecheck failed -- with no analyzer errors"); \
            }                                                           \
        }                                                               \
    } while(0)


#define ck_assert_analyzer_errors(info_, expected_arr_)					\
    ck_assert_analyzer_errors_impl(                                       \
        info_,                                                          \
        expected_arr_,                                                  \
        sizeof(expected_arr_)/sizeof(struct info_msg),                  \
        __FILE__, __LINE__)
bool ck_assert_analyzer_errors_impl(struct info_ctx *info,
                                    struct info_msg *errors,
                                    unsigned num,
                                    const char *filename,
                                    unsigned int line);


#define ck_assert_typecheck_with_messages(d_, success_, expected_msgs_) \
    do {                                                                \
        testsupport_typecheck_prepare(d_);                              \
        ck_assert_msg(success_ == analyzer_typecheck((d_)->front.analyzer), \
                      "Unexpected typecheck result");                   \
        ck_assert_analyzer_errors((d_)->front.info, expected_msgs_);    \
    } while(0)



#endif
