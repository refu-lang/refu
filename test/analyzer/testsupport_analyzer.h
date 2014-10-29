#ifndef LFR_TESTSUPPORT_ANALYZER_H
#define LFR_TESTSUPPORT_ANALYZER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <inplocation.h>
#include <parser/parser.h>
#include <lexer/lexer.h>



struct ast_node;

struct analyzer_testdriver {
    //TODO: Think, if we will need this additional driver for analyzer tests
    int something;
};
struct analyzer_testdriver *get_analyzer_testdriver();

void setup_analyzer_tests();
void teardown_analyzer_tests();

#define testsupport_analyzer_prepare(driver_)                     \
    do {                                                          \
        ck_assert(lexer_scan((driver_)->front.lexer));            \
        ck_assert(parser_process_file((driver_)->front.parser));        \
        (driver_)->front.analyzer->root = parser_yield_ast_root((driver_)->front.parser); \
    } while (0)

#define testsupport_symbol_table_add_node(st_, driver_, id_, node_)     \
    do {                                                                \
        ck_assert(symbol_table_add_node(st_, (driver_)->front.analyzer, id_, node_)); \
    } while (0)

#define testsupport_symbol_table_lookup_node(st_, idstring_, retval_, first_st) \
    do {                                                                \
        bool at_first_st;                                               \
        retval_ = symbol_table_lookup_node(st_, idstring_, &at_first_st); \
        ck_assert_msg(retval_, "Symbol table lookup failed");           \
        ck_assert_msg(first_st == at_first_st,                          \
                      "symbol found but not, in the expected table");   \
    } while(0);

#define testsupport_symbol_table_lookup_record(st_, idstring_, retval_, first_st) \
    do {                                                                \
        bool at_first_st;                                               \
        retval_ = symbol_table_lookup_record(st_, idstring_, &at_first_st); \
        ck_assert_msg(retval_, "Symbol table lookup failed");           \
        ck_assert_msg(first_st == at_first_st,                          \
                      "symbol found but not, in the expected table");   \
    } while(0);

#endif
