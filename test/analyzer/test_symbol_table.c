#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include <ast/ast.h>
#include <ast/block.h>
#include <ast/identifier.h>

#include "../../src/analyzer/symbol_table_creation.h"

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST(test_symbol_table_add) {
    struct symbol_table st;
    struct ast_node *n;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a = 4.214\n"
        "var_2 = 5 + a"
    );
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("var_2");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(symbol_table_init(&st, d->front.analyzer));

    struct ast_node *id1 = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                                "a");
    struct ast_node *id2 = front_testdriver_generate_identifier(d, 1, 0, 1, 0,
                                                                "var_2");

    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id1), id1));
    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id2), id2));

    n = symbol_table_lookup_node(&st, &id1s);
    ck_assert(n == id1);
    n = symbol_table_lookup_node(&st, &id2s);
    ck_assert(n == id2);

    symbol_table_deinit(&st);
}END_TEST

START_TEST(test_symbol_table_add_existing) {
    struct symbol_table st;
    struct symbol_table_record *rec;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a = 4.214\n"
        "var_2 = 5 + a"
    );
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("var_2");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(symbol_table_init(&st, d->front.analyzer));

    struct ast_node *id1 = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                                "a");
    struct ast_node *id2 = front_testdriver_generate_identifier(d, 1, 0, 1, 0,
                                                                "var_2");

    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id1), id1));
    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id2), id2));
    ck_assert(!symbol_table_add_node(&st, ast_identifier_str(id2), id2));

    rec = symbol_table_lookup_record(&st, &id1s);
    ck_assert(symbol_table_record_type(rec) == id1);
    rec = symbol_table_lookup_record(&st, &id2s);
    ck_assert(symbol_table_record_type(rec) == id2);

    symbol_table_deinit(&st);
}END_TEST

START_TEST(test_symbol_table_lookup_non_existing) {
    struct symbol_table st;
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("I_dont_exist");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("neither_do_I");
    static const struct RFstring s = RF_STRING_STATIC_INIT("program");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(symbol_table_init(&st, d->front.analyzer));

    struct ast_node *id1 = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                                "a");
    struct ast_node *id2 = front_testdriver_generate_identifier(d, 1, 0, 1, 0,
                                                                "var_2");

    // add something so that we don't check against an empty symbol table
    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id1), id1));
    ck_assert(symbol_table_add_node(&st, ast_identifier_str(id2), id2));


    ck_assert(symbol_table_lookup_record(&st, &id1s) == NULL);
    ck_assert(symbol_table_lookup_record(&st, &id2s) == NULL);

    symbol_table_deinit(&st);
}END_TEST


struct st_test_record {
    struct RFstring s;
    struct ast_node *n;
};

static struct st_test_record ids_arr[] = {
    {.s=RF_STRING_STATIC_INIT("test_identifier1")},
    {.s=RF_STRING_STATIC_INIT("test_identifier2")},
    {.s=RF_STRING_STATIC_INIT("test_identifier3")},
    {.s=RF_STRING_STATIC_INIT("test_identifier4")},
    {.s=RF_STRING_STATIC_INIT("test_identifier5")},
    {.s=RF_STRING_STATIC_INIT("test_identifier6")},
    {.s=RF_STRING_STATIC_INIT("test_identifier7")},
    {.s=RF_STRING_STATIC_INIT("test_identifier8")},
    {.s=RF_STRING_STATIC_INIT("test_identifier9")},
    {.s=RF_STRING_STATIC_INIT("test_identifier10")},
    {.s=RF_STRING_STATIC_INIT("test_identifier11")},
    {.s=RF_STRING_STATIC_INIT("test_identifier12")},
    {.s=RF_STRING_STATIC_INIT("test_identifier13")},
    {.s=RF_STRING_STATIC_INIT("test_identifier14")},
    {.s=RF_STRING_STATIC_INIT("test_identifier15")},
    {.s=RF_STRING_STATIC_INIT("test_identifier16")},
    {.s=RF_STRING_STATIC_INIT("test_identifier17")},
    {.s=RF_STRING_STATIC_INIT("test_identifier18")},
    {.s=RF_STRING_STATIC_INIT("test_identifier19")},
    {.s=RF_STRING_STATIC_INIT("test_identifier20")},
    {.s=RF_STRING_STATIC_INIT("test_identifier21")},
    {.s=RF_STRING_STATIC_INIT("test_identifier22")},
    {.s=RF_STRING_STATIC_INIT("test_identifier23")},
    {.s=RF_STRING_STATIC_INIT("test_identifier24")},
    {.s=RF_STRING_STATIC_INIT("test_identifier25")},
    {.s=RF_STRING_STATIC_INIT("test_identifier26")},
    {.s=RF_STRING_STATIC_INIT("test_identifier27")},
    {.s=RF_STRING_STATIC_INIT("test_identifier28")},
    {.s=RF_STRING_STATIC_INIT("test_identifier29")},
    {.s=RF_STRING_STATIC_INIT("test_identifier30")},
    {.s=RF_STRING_STATIC_INIT("test_identifier31")},
    {.s=RF_STRING_STATIC_INIT("test_identifier32")},
    {.s=RF_STRING_STATIC_INIT("test_identifier33")},
    {.s=RF_STRING_STATIC_INIT("test_identifier34")},
    {.s=RF_STRING_STATIC_INIT("test_identifier35")},
    {.s=RF_STRING_STATIC_INIT("test_identifier36")},
    {.s=RF_STRING_STATIC_INIT("test_identifier37")},
    {.s=RF_STRING_STATIC_INIT("test_identifier38")},
    {.s=RF_STRING_STATIC_INIT("test_identifier39")},
    {.s=RF_STRING_STATIC_INIT("test_identifier40")},
    {.s=RF_STRING_STATIC_INIT("test_identifier41")},
    {.s=RF_STRING_STATIC_INIT("test_identifier42")},
    {.s=RF_STRING_STATIC_INIT("test_identifier43")},
    {.s=RF_STRING_STATIC_INIT("test_identifier44")},
    {.s=RF_STRING_STATIC_INIT("test_identifier45")},
    {.s=RF_STRING_STATIC_INIT("test_identifier46")},
    {.s=RF_STRING_STATIC_INIT("test_identifier47")},
    {.s=RF_STRING_STATIC_INIT("test_identifier48")},
    {.s=RF_STRING_STATIC_INIT("test_identifier49")},
    {.s=RF_STRING_STATIC_INIT("test_identifier50")},
    {.s=RF_STRING_STATIC_INIT("test_identifier51")},
    {.s=RF_STRING_STATIC_INIT("test_identifier52")},
    {.s=RF_STRING_STATIC_INIT("test_identifier53")},
    {.s=RF_STRING_STATIC_INIT("test_identifier54")},
    {.s=RF_STRING_STATIC_INIT("test_identifier55")},
    {.s=RF_STRING_STATIC_INIT("test_identifier56")},
    {.s=RF_STRING_STATIC_INIT("test_identifier57")},
    {.s=RF_STRING_STATIC_INIT("test_identifier58")},
    {.s=RF_STRING_STATIC_INIT("test_identifier59")},
    {.s=RF_STRING_STATIC_INIT("test_identifier60")},
    {.s=RF_STRING_STATIC_INIT("test_identifier61")},
    {.s=RF_STRING_STATIC_INIT("test_identifier62")},
    {.s=RF_STRING_STATIC_INIT("test_identifier63")},
    {.s=RF_STRING_STATIC_INIT("test_identifier64")},
    {.s=RF_STRING_STATIC_INIT("test_identifier65")},
    {.s=RF_STRING_STATIC_INIT("test_identifier66")},
    {.s=RF_STRING_STATIC_INIT("test_identifier67")},
    {.s=RF_STRING_STATIC_INIT("test_identifier68")},
    {.s=RF_STRING_STATIC_INIT("test_identifier69")},
    {.s=RF_STRING_STATIC_INIT("test_identifier70")},
    {.s=RF_STRING_STATIC_INIT("test_identifier71")},
    {.s=RF_STRING_STATIC_INIT("test_identifier72")},
    {.s=RF_STRING_STATIC_INIT("test_identifier73")},
    {.s=RF_STRING_STATIC_INIT("test_identifier74")},
    {.s=RF_STRING_STATIC_INIT("test_identifier75")},
    {.s=RF_STRING_STATIC_INIT("test_identifier76")},
    {.s=RF_STRING_STATIC_INIT("test_identifier77")},
    {.s=RF_STRING_STATIC_INIT("test_identifier78")},
    {.s=RF_STRING_STATIC_INIT("test_identifier79")},
    {.s=RF_STRING_STATIC_INIT("test_identifier80")},
    {.s=RF_STRING_STATIC_INIT("test_identifier81")},
    {.s=RF_STRING_STATIC_INIT("test_identifier82")},
    {.s=RF_STRING_STATIC_INIT("test_identifier83")},
    {.s=RF_STRING_STATIC_INIT("test_identifier84")},
    {.s=RF_STRING_STATIC_INIT("test_identifier85")},
    {.s=RF_STRING_STATIC_INIT("test_identifier86")},
    {.s=RF_STRING_STATIC_INIT("test_identifier87")},
    {.s=RF_STRING_STATIC_INIT("test_identifier88")},
    {.s=RF_STRING_STATIC_INIT("test_identifier89")},
    {.s=RF_STRING_STATIC_INIT("test_identifier90")},
    {.s=RF_STRING_STATIC_INIT("test_identifier91")},
    {.s=RF_STRING_STATIC_INIT("test_identifier92")},
    {.s=RF_STRING_STATIC_INIT("test_identifier93")},
    {.s=RF_STRING_STATIC_INIT("test_identifier94")},
    {.s=RF_STRING_STATIC_INIT("test_identifier95")},
    {.s=RF_STRING_STATIC_INIT("test_identifier96")},
    {.s=RF_STRING_STATIC_INIT("test_identifier97")},
    {.s=RF_STRING_STATIC_INIT("test_identifier98")},
    {.s=RF_STRING_STATIC_INIT("test_identifier99")},
    {.s=RF_STRING_STATIC_INIT("test_identifier100")},
};

static struct ast_node *generate_test_identifier(struct front_testdriver *d,
                                                 unsigned int i)
{
    return front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                rf_string_data(&ids_arr[i].s));
}

START_TEST(test_symbol_table_many_symbols) {
    size_t ids_num;
    struct ast_node *n;
    struct symbol_table_record *rec;
    unsigned int i;
    struct symbol_table st;
    static const struct RFstring s = RF_STRING_STATIC_INIT("program");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);
    ck_assert(symbol_table_init(&st, d->front.analyzer));

    ids_num = sizeof(ids_arr) / sizeof(struct st_test_record);
    for (i = 0; i < ids_num; i ++) {
        n = generate_test_identifier(d, i);
        ck_assert_msg(n, "Could not generate a test identifier");
        ids_arr[i].n = n;
        ck_assert_msg(symbol_table_add_node(&st, &ids_arr[i].s, n),
                      "Could not add %u/%zu generated identifier to the symbol table",
                      i, ids_num);
    }

    for (i = 0; i < ids_num; i ++) {
            rec = symbol_table_lookup_record(&st, &ids_arr[i].s);
            ck_assert_msg(symbol_table_record_type(rec) == ids_arr[i].n,
                          "Generated identifier lookup mismatch");
    }

    symbol_table_deinit(&st);
} END_TEST

START_TEST(test_block_symbol_table) {
    struct symbol_table *st;
    struct ast_node *n;
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("b");
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "   a:i64\n"
        "   b:u32\n"
        "   c = a + b\n"
        "}\n"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    testsupport_parser_xidentifier_create_simple(id1, &d->front.file,
                                                 1, 5, 1, 7);
    testsupport_parser_xidentifier_create_simple(id2, &d->front.file,
                                                 2, 5, 2, 7);

    testsupport_analyzer_prepare(d);
    ck_assert(analyzer_create_symbol_tables(d->front.analyzer));

    struct ast_node *block = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(block, "block node was not found");
    st = ast_block_symbol_table_get(block);

    testsupport_symbol_table_lookup_node(st, &id1s, n);
    check_ast_match(n, id1, &d->front.file);
    testsupport_symbol_table_lookup_node(st, &id2s, n);
    check_ast_match(n, id2, &d->front.file);

    ast_node_destroy(id1);
    ast_node_destroy(id2);
} END_TEST

Suite *analyzer_symboltable_suite_create(void)
{
    Suite *s = suite_create("analyzer_symbol_Table");

    TCase *st1 = tcase_create("analyzer_symbol_table_simple_mechanics");
    tcase_add_checked_fixture(st1,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);

    tcase_add_test(st1, test_symbol_table_add);
    tcase_add_test(st1, test_symbol_table_add_existing);
    tcase_add_test(st1, test_symbol_table_lookup_non_existing);
    tcase_add_test(st1, test_symbol_table_many_symbols);

    TCase *st2 = tcase_create("analyzer_symbol_table_populate");
    tcase_add_checked_fixture(st2,
                              setup_analyzer_tests,
                              teardown_analyzer_tests);
    tcase_add_test(st2, test_block_symbol_table);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}


