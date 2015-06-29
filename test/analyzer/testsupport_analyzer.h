#ifndef LFR_TESTSUPPORT_ANALYZER_H
#define LFR_TESTSUPPORT_ANALYZER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <inplocation.h>
#include <parser/parser.h>
#include <lexer/lexer.h>

#include <types/type.h>
#include <types/type_comparisons.h>
#include <analyzer/typecheck.h>


struct ast_node;

struct analyzer_testdriver {
    //! A buffer of types for quick type checks
    //! and easy freeing at test teardown
    struct {darray(struct type*);} types;
};
struct analyzer_testdriver *get_analyzer_testdriver();

bool analyzer_testdriver_init(struct analyzer_testdriver *d);
void analyzer_testdriver_deinit(struct analyzer_testdriver *d);

void setup_analyzer_tests();
void setup_analyzer_tests_no_stdlib();
void setup_analyzer_tests_with_filelog();
void setup_analyzer_tests_no_source();
void teardown_analyzer_tests();

#define testsupport_show_front_errors(msg_)                             \
    do {                                                                \
        struct RFstringx *tmp_ = front_testdriver_geterrors(get_front_testdriver()); \
        if (tmp_) {                                                     \
            ck_abort_msg(msg_ " with errors:\n"RF_STR_PF_FMT,           \
                         RF_STR_PF_ARG(tmp_));                          \
        } else {                                                        \
            ck_abort_msg(msg_" with no specific errors");               \
        }                                                               \
    } while(0)

#define testsupport_scan_and_parse()                                    \
    do {                                                                \
        if (!compiler_preprocess_fronts()) {                            \
            testsupport_show_front_errors("Scanning/parsing failed");   \
        }                                                               \
} while (0)

#define testsupport_analyzer_prepare()                                  \
    do {                                                                \
        testsupport_scan_and_parse();                                   \
        ck_assert(module_symbol_table_init(front_testdriver_module())); \
    } while(0)

#define testsupport_symbol_table_add_node(st_, id_, node_)              \
    do {                                                                \
        ck_assert(symbol_table_add_node(st_, front_testdriver_module(), id_, node_)); \
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
        ck_assert_msg(type_compare(t1_, t2_, TYPECMP_GENERIC),          \
                      "expected type mismatch."RF_STR_PF_FMT,            \
                      RF_STR_PF_ARG(typecmp_ctx_get_error()));          \
    } while(0)


struct type *testsupport_analyzer_type_create_elementary(enum elementary_type etype,
                                                         bool constant);

struct type *testsupport_analyzer_type_create_operator(enum typeop_type type,
                                                       struct type *left,
                                                       struct type *right);

struct type *testsupport_analyzer_type_create_defined(const struct RFstring *name,
                                                      struct type *type);

struct type *testsupport_analyzer_type_create_leaf(const struct RFstring *id,
                                                   struct type *type);

struct type *testsupport_analyzer_type_create_function(struct type *arg,
                                                       struct type *ret);

/* -- general analyzer/front context of the compiler support*/

#define ck_assert_analyzer_errors(expected_arr_) \
    ck_assert_analyzer_errors_impl(                     \
        get_front_testdriver()->current_front->info,    \
        expected_arr_,                                  \
        sizeof(expected_arr_)/sizeof(struct info_msg),  \
        __FILE__, __LINE__)
bool ck_assert_analyzer_errors_impl(struct info_ctx *info,
                                    struct info_msg *errors,
                                    unsigned num,
                                    const char *filename,
                                    unsigned int line);

/* -- typecheck related support -- */

//! Assert all of the front context processing including typechecking is done
#define ck_assert_typecheck_ok()                                        \
    do {                                                                \
        testsupport_scan_and_parse();                                   \
        if (!compiler_analyze()) {                                      \
            testsupport_show_front_errors("Typechecking failed");       \
        }                                                               \
    } while(0)

#define ck_assert_typecheck_with_messages(success_, expected_msgs_)     \
    do {                                                                \
        testsupport_scan_and_parse();                                   \
        if (success_ != compiler_analyze()) {                           \
            testsupport_show_front_errors("Typechecking result was unexpected"); \
        }                                                               \
        ck_assert_analyzer_errors(expected_msgs_);                      \
    } while(0)

#endif
