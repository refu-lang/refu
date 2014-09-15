#ifndef LFR_TESTSUPPORT_PARSER_H
#define LFR_TESTSUPPORT_PARSER_H

#include <Preprocessor/rf_xmacro_argcount.h>

/**
 * A utility testing macro to generate an ast_node whose _create() accepts
 * a start and end location mark
 */
#define testsupport_parser_node_create(...)                             \
    RF_SELECT_FUNC_IF_NARGGT(i_testsupport_parser_node_create, 7, __VA_ARGS__) \
        
#define i_testsupport_parser_node_create1(node_, type_,                 \
                                          file_, sl_, sc_, el_, ec_, ...) \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(file_, sl_, sc_, el_, ec_); \
        node_ = ast_##type_##_create(&temp_location_.start,\
                                     &temp_location_.end, __VA_ARGS__);  \
    } while(0)                                                         

#define i_testsupport_parser_node_create0(node_, type_,                 \
                                          file_, sl_, sc_, el_, ec_)    \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(file_, sl_, sc_, el_, ec_); \
        node_ = ast_##type_##_create(&temp_location_.start,             \
                                     &temp_location_.end);              \
    } while(0)                                                         

/**
 * A utility testing macro to generate an identifier at a location
 */    
#define testsupport_parser_identifier_create(node_,                   \
                                               file_, sl_, sc_, el_, ec_) \
    struct ast_node *node_;                                             \
    do {                                                                \
        struct inplocation temp_location_ = LOC_INIT(file_, sl_, sc_, el_, ec_); \
        node_ = ast_identifier_create(&temp_location_);                 \
    } while(0)                                                             



#define testsupport_parser_prepare(driver_)                     \
    do {                                                        \
        ck_assert(lexer_scan((driver_)->front.lexer));          \
        lexer_renounce_own_identifiers((driver_)->front.lexer); \
    } while (0)

/**
 * A utility testing macro used to test if the parser succesfully does an
 * accept
 */
#define ck_test_parse_as(node_, type_, driver_,  node_name, target_)    \
        do {                                                            \
            testsupport_parser_prepare(driver_);                        \
            node_ = parser_acc_##type_((driver_)->front.parser);        \
            ck_assert_parsed_node(node_, driver_, "Could not parse "node_name); \
            check_ast_match(n, target_, &(driver_)->front.file);        \
        } while (0)


#define ck_assert_parsed_node(n_, d_, msg_)                             \
        do {                                                            \
            if (!(n_)) {                                                \
                struct RFstringx *tmp_ = front_testdriver_geterrors(d_); \
                if (tmp_) {                                             \
                    ck_abort_msg(msg_" -- with parser errors\n"RF_STR_PF_FMT, \
                                 RF_STR_PF_ARG(tmp_));                  \
                } else {                                                \
                    ck_abort_msg(msg_" -- with no parser errors");      \
                }                                                       \
            }                                                           \
        } while(0)

#define ck_assert_parser_error(d_, err_)                                \
        do {                                                            \
            static const struct RFstring i_tmps_ = RF_STRING_STATIC_INIT(err_); \
            struct RFstringx *i_tmp_ = front_testdriver_geterrors(d_);  \
            if (!i_tmp_) {                                              \
                ck_abort_msg("Expected parsing error but none found");  \
            }                                                           \
            ck_assert_msg(                                              \
                rf_string_equal(&i_tmps_, i_tmp_),                      \
                "Expected parsing error does not match. Expected:\n"    \
                RF_STR_PF_FMT"\nGot:\n"RF_STR_PF_FMT,                   \
                RF_STR_PF_ARG(&i_tmps_), RF_STR_PF_ARG(i_tmp_));        \
        } while(0)

#endif
