#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H

#include <stdbool.h>
#include <check.h>
#include <rflib/preprocessor/rf_xmacro_argcount.h>
#include <ast/identifier.h>

struct front_testdriver;

/**
 * A utility testing macro to generate an ast_node whose _create() accepts
 * a start and end location mark
 */
#define testsupport_parser_node_create(...)                             \
    RF_SELECT_FUNC_IF_NARGGT(i_testsupport_parser_node_create, 6, __VA_ARGS__)

#define i_testsupport_parser_node_create1(node_, type_, sl_, sc_, el_, ec_, ...)      \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(get_front_testdriver()->current_front->file, sl_, sc_, el_, ec_); \
        node_ = ast_##type_##_create(&temp_location_.start,\
                                     &temp_location_.end, __VA_ARGS__);  \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while(0)

#define i_testsupport_parser_node_create0(node_, type_, sl_, sc_, el_, ec_) \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_##type_##_create(&temp_location_.start,             \
                                     &temp_location_.end);              \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
} while(0)

/**
 * Utility testing macros to generate a typedesc node at location with its
 * direct child description at the same line
 */
#define testsupport_parser_typedesc_create(...)                         \
    RF_SELECT_FUNC_IF_NARGGT(i_testsupport_parser_typedesc_create, 6, __VA_ARGS__)
#define testsupport_parser_typedesc_create_xidentifier(node_,           \
                                                       sl_, sc_, el_,   \
                                                       ec_)             \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_typedesc_create(                                    \
            ast_xidentifier_create(                                     \
                &temp_location_.start, &temp_location_.end,             \
                testsupport_parser_identifier_create(sl_, sc_, el_,ec_), \
                false, false, NULL)                                     \
        );                                                              \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while (0)
    
#define i_testsupport_parser_typedesc_create1(node_,                    \
                                              sl_, sc_, el_,            \
                                              ec_, type_, ...)          \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_typedesc_create(                                    \
            ast_##type_##_create(&temp_location_.start, &temp_location_.end, __VA_ARGS__) \
        );                                                              \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while (0)

#define i_testsupport_parser_typedesc_create0(node_,                    \
                                              sl_, sc_, el_,            \
                                              ec_, type_)               \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_typedesc_create(                                    \
            ast_##type_##_create(&temp_location_.start, &temp_location_.end) \
        );                                                              \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while (0)

/**
 * A utility testing macro to generate a constant node at a location
 */
#define testsupport_parser_constant_create(node_,                       \
                                           sl_, sc_, el_,               \
                                           ec_, type_, value_)          \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_constant_create_##type_(&temp_location_, value_);   \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while (0)

/**
 * A utility testing macro to generate a string literal at a location
 */
#define testsupport_parser_string_literal_create(node_,                 \
                                                 sl_, sc_, el_,         \
                                                 ec_)                   \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
        node_ = ast_string_literal_create(&temp_location_);             \
        node_->state = AST_NODE_STATE_AFTER_PARSING;                    \
    } while (0)

/**
 * A utility testing macro to generate a block at a location
 */
#define testsupport_parser_block_create(node_,                          \
                                        sl_, sc_, el_,                  \
                                        ec_)                            \
        struct ast_node *node_;                                         \
        do {                                                            \
        struct inplocation temp_location_ = LOC_INIT(                   \
            get_front_testdriver()->current_front->file,                \
            sl_,                                                        \
            sc_,                                                        \
            el_,                                                        \
            ec_                                                         \
        );                                                              \
            node_ = ast_block_create();                                 \
            ast_node_set_start(node_, &temp_location_.start);           \
            ast_node_set_end(node_, &temp_location_.end);               \
            node_->state = AST_NODE_STATE_AFTER_PARSING;                \
        } while (0)

/**
 * A utility testing function to generate an identifier at a location
 */
struct ast_node *testsupport_parser_identifier_create(
    unsigned int sline,
    unsigned int scol,
    unsigned int eline,
    unsigned int ecol
);

/**
 * A utility test macro to help create an xidentifier node wrapped over
 * a simple identifier
 */
#define testsupport_parser_xidentifier_create_simple(node_,             \
                                                     sl_, sc_, el_, ec_) \
    testsupport_parser_node_create(node_, xidentifier,                  \
                                   sl_, sc_, el_, ec_,                  \
                                   testsupport_parser_identifier_create( \
                                       sl_, sc_, el_, ec_),             \
                                   false, false, NULL)

#define testsupport_parser_prepare()                                    \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
    } while (0)


// convenience function, basically a copy of parser's finalize parsing, without
// the module related logic
void i_test_finalize_parsing(struct ast_node *n);

/**
 * A utility testing macro used to test if the parser succesfully does an
 * accept.
 */
#define ck_test_parse_as(...) \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_test_parse_as, 4, __VA_ARGS__)

#define i_ck_test_parse_as1(node_, type_, node_name, target_, ...)      \
    do {                                                                \
        testsupport_parser_prepare();                                   \
        node_ = ast_parser_acc_##type_(front_testdriver_ast_parser(), __VA_ARGS__); \
        ck_assert_parsed_node(node_, "Could not parse "node_name);      \
        check_ast_match(n, target_, get_front_testdriver()->current_front->file); \
        i_test_finalize_parsing(node_);                                 \
    } while (0)

#define i_ck_test_parse_as0(node_, type_,  node_name, target_)          \
    do {                                                                \
        testsupport_parser_prepare();                                   \
        node_ = ast_parser_acc_##type_(front_testdriver_ast_parser());      \
        ck_assert_parsed_node(node_, "Could not parse "node_name);      \
        check_ast_match(n, target_, get_front_testdriver()->current_front->file); \
        i_test_finalize_parsing(node_);                                 \
    } while (0)

#define ck_test_parse_root(node_, target_)                              \
    do {                                                                \
        testsupport_parser_prepare();                                   \
        ast_parser_process_file(front_testdriver_ast_parser());         \
        node_ = front_testdriver_ast_parser()->root;                    \
        ck_assert_parsed_node(node_, "Could not parse root node");      \
        check_ast_match(n, target_, get_front_testdriver()->current_front->file); \
        i_test_finalize_parsing(node_);                                 \
    } while (0)

/**
 * A utility testing macro used to test if we detect parsing failure properly
 * with syntax errors
 */
#define ck_test_fail_parse_as(...)                                      \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_test_fail_parse_as, 1, __VA_ARGS__)

#define i_ck_test_fail_parse_as1(type_, ...)                            \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
        ck_assert_msg(NULL == ast_parser_acc_##type_(front_testdriver_ast_parser(), __VA_ARGS__), \
                      "parsing "#type_"should have failed");            \
        ck_assert_msg(ast_parser_has_syntax_error(front_testdriver_ast_parser()), \
                      "a syntax error should have been reported");      \
    } while (0)

#define i_ck_test_fail_parse_as0(type_, ...)                            \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
        ck_assert_msg(NULL == ast_parser_acc_##type_(front_testdriver_ast_parser()), \
                      "parsing "#type_"should have failed");            \
        ck_assert_msg(ast_parser_has_syntax_error(front_testdriver_ast_parser()), \
                      "a syntax error should have been reported");      \
    } while (0)

/**
 * A utility testing macro used to test if we detect parsing failure properly
 * without any syntax errors
 */
#define ck_test_fail_parse_noerr_as(...)                                      \
    RF_SELECT_FUNC_IF_NARGGT(i_ck_test_fail_parse_noerr_as, 1, __VA_ARGS__)

#define i_ck_test_fail_parse_noerr_as1(type_, ...)                            \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
        ck_assert_msg(NULL == ast_parser_acc_##type_(front_testdriver_ast_parser(), __VA_ARGS__), \
                      "parsing "#type_"should have failed");            \
        ck_assert_msg(!ast_parser_has_syntax_error(get_front_testdriver()->current_front->parser), \
                      "no syntax error should have been reported");      \
    } while (0)

#define i_ck_test_fail_parse_noerr_as0(type_, ...)                            \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
        ck_assert_msg(NULL == ast_parser_acc_##type_(front_testdriver_ast_parser()), \
                      "parsing "#type_"should have failed");            \
        ck_assert_msg(!ast_parser_has_syntax_error(front_testdriver_ast_parser()), \
                      "no syntax error should have been reported");      \
    } while (0)

/**
 * Utility testing macro for testing parsing failure of parsing an entire file
 */
#define ck_test_fail_parse_file()                                       \
    do {                                                                \
        ck_assert(lexer_scan(get_front_testdriver()->current_front->lexer)); \
        ck_assert_msg(!ast_parser_process_file(front_testdriver_ast_parser()), \
                      "parsing should have failed");                    \
        ck_assert_msg(ast_parser_has_syntax_error(front_testdriver_ast_parser()), \
                      "a syntax error should have been reported");      \
    } while (0)


#define ck_assert_parsed_node(n_,  msg_)                                \
    do {                                                                \
        if (!(n_)) {                                                    \
            struct RFstringx *tmp_ = front_testdriver_geterrors(get_front_testdriver()); \
            if (tmp_) {                                                 \
                ck_abort_msg(                                           \
                    msg_" -- with parser errors\n"RFS_PF,               \
                    RFS_PA(tmp_)                                        \
                );                                                      \
            } else {                                                    \
                ck_abort_msg(msg_" -- with no parser errors");          \
            }                                                           \
        }                                                               \
    } while(0)

#define ck_assert_parser_errors(expected_arr_)                          \
        ck_assert_parser_errors_impl(                                   \
            get_front_testdriver()->current_front->info,                \
            expected_arr_,                                              \
            sizeof(expected_arr_)/sizeof(struct info_msg),              \
            __FILE__, __LINE__)

struct info_msg;
struct info_ctx;
bool ck_assert_parser_errors_impl(struct info_ctx *info,
                                  struct info_msg *errors,
                                  unsigned num,
                                  const char *filename,
                                  unsigned int line);
#endif
