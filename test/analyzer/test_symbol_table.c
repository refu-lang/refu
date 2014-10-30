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
#include <ast/function.h>
#include <ast/type.h>

#include "../../src/analyzer/symbol_table_creation.h"

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

/* -- simple symbol table functionality tests -- */

START_TEST(test_symbol_table_add) {
    // just a simple addition test, makes no sense functionality wise
    struct symbol_table st;
    struct ast_node *n;
    bool at_first;
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

    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id1), id1);
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id2), id2);

    n = symbol_table_lookup_node(&st, &id1s, &at_first);
    ck_assert(n == id1);
    ck_assert(at_first);
    n = symbol_table_lookup_node(&st, &id2s, &at_first);
    ck_assert(n == id2);
    ck_assert(at_first);

    symbol_table_deinit(&st);
}END_TEST

START_TEST(test_symbol_table_add_existing) {
    struct symbol_table st;
    bool at_first;
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

    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id1), id1);
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id2), id2);
    ck_assert(!symbol_table_add_node(&st, d->front.analyzer,
                                     ast_identifier_str(id2), id2));

    rec = symbol_table_lookup_record(&st, &id1s, &at_first);
    ck_assert(symbol_table_record_node(rec) == id1);
    ck_assert(at_first);
    rec = symbol_table_lookup_record(&st, &id2s, &at_first);
    ck_assert(symbol_table_record_node(rec) == id2);
    ck_assert(at_first);

    symbol_table_deinit(&st);
}END_TEST

START_TEST(test_symbol_table_lookup_non_existing) {
    struct symbol_table st;
    bool at_first;
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
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id1), id1);
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id2), id2);


    ck_assert(symbol_table_lookup_record(&st, &id1s, &at_first) == NULL);
    ck_assert(symbol_table_lookup_record(&st, &id2s, &at_first) == NULL);

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
    bool at_first;
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
        ck_assert_msg(symbol_table_add_node(&st, d->front.analyzer,
                                            &ids_arr[i].s, n),
                      "Could not add %u/%zu generated identifier to the symbol table",
                      i, ids_num);
    }

    for (i = 0; i < ids_num; i ++) {
        rec = symbol_table_lookup_record(&st, &ids_arr[i].s, &at_first);
        ck_assert(at_first);
        ck_assert_msg(symbol_table_record_node(rec) == ids_arr[i].n,
                      "Generated identifier lookup mismatch");
    }

    symbol_table_deinit(&st);
} END_TEST


/* -- symbol table creation testing for specific nodes -- */


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

    testsupport_parser_xidentifier_create_simple(id2, &d->front.file,
                                                 1, 5, 1, 7);
    testsupport_parser_xidentifier_create_simple(id4, &d->front.file,
                                                 2, 5, 2, 7);

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_create_symbol_tables(d->front.analyzer));

    struct ast_node *block = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(block, "block node was not found");
    st = ast_block_symbol_table_get(block);

    testsupport_symbol_table_lookup_node(st, &id1s, n, true);
    check_ast_match(n, id2, &d->front.file);
    testsupport_symbol_table_lookup_node(st, &id2s, n, true);
    check_ast_match(n, id4, &d->front.file);

    ast_node_destroy(id2);
    ast_node_destroy(id4);
} END_TEST

START_TEST(test_fndecl_symbol_table) {
    struct symbol_table *st;
    struct symbol_table_record *rec;
    static const struct RFstring names = RF_STRING_STATIC_INIT("do_sth");
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("var");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("s");
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "fn do_sth(var:u64, s:string) -> u32 { }"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);


    struct ast_node *fn_name = testsupport_parser_identifier_create(
        &d->front.file,
        0, 3, 0, 8);

    struct ast_node *id1 = testsupport_parser_identifier_create(
        &d->front.file,
        0, 10, 0, 12);
    testsupport_parser_xidentifier_create_simple(id2, &d->front.file,
                                                 0, 14, 0, 16);
    testsupport_parser_node_create(t1, typedesc, &d->front.file,
                                   0, 10, 0, 16, id1, id2);


    struct ast_node *id3 = testsupport_parser_identifier_create(
        &d->front.file,
        0, 19, 0, 19);
    testsupport_parser_xidentifier_create_simple(id4, &d->front.file,
                                                 0, 21, 0, 26);
    testsupport_parser_node_create(t2, typedesc, &d->front.file,
                                   0, 19, 0, 26, id3, id4);
    testsupport_parser_node_create(op1, typeop, &d->front.file, 0, 10, 0, 26,
                                   TYPEOP_PRODUCT, t1, t2);

    testsupport_parser_xidentifier_create_simple(id5, &d->front.file,
                                                 0, 32, 0, 34);

    testsupport_parser_node_create(fn, fndecl, &d->front.file, 0, 0, 0, 34,
                                   fn_name,
                                   NULL,
                                   op1,
                                   id5
    );

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_create_symbol_tables(d->front.analyzer));

    struct ast_node *fnimpl = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(fnimpl, "fnimpl node was not found");
    st = ast_fndecl_symbol_table_get(ast_fnimpl_fndecl_get(fnimpl));

    testsupport_symbol_table_lookup_record(st, &id1s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_UINT_64);
    check_ast_match(symbol_table_record_node(rec), id2, &d->front.file);

    testsupport_symbol_table_lookup_record(st, &id2s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_STRING);
    check_ast_match(symbol_table_record_node(rec), id4, &d->front.file);

    testsupport_symbol_table_lookup_record(st, &names, rec, false);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_FUNCTION);
    check_ast_match(symbol_table_record_node(rec), fn, &d->front.file);

    ast_node_destroy(fn);
} END_TEST


START_TEST(test_typedecl_symbol_table) {
    struct symbol_table *st;
    struct symbol_table_record *rec;
    static const struct RFstring names = RF_STRING_STATIC_INIT("person");
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("name");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("age");
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type person {\n"
        "name:string, age:u16\n"
        "}"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    struct ast_node *type_name = testsupport_parser_identifier_create(
        &d->front.file,
        0, 5, 0, 10);
    struct ast_node *id1 = testsupport_parser_identifier_create(
        &d->front.file,
        1, 0, 1, 3);
    testsupport_parser_xidentifier_create_simple(id2, &d->front.file,
                                                 1, 5, 1, 10);
    testsupport_parser_node_create(t1, typedesc, &d->front.file,
                                   1, 0, 1, 10, id1, id2);


    struct ast_node *id3 = testsupport_parser_identifier_create(
        &d->front.file,
        1, 13, 1, 15);
    testsupport_parser_xidentifier_create_simple(id4, &d->front.file,
                                                 1, 17, 1, 19);
    testsupport_parser_node_create(t2, typedesc, &d->front.file,
                                   1, 13, 1, 19, id3, id4);

    testsupport_parser_node_create(op1, typeop, &d->front.file, 1, 0, 1, 19,
                                   TYPEOP_PRODUCT, t1, t2);
    testsupport_parser_node_create(tdecl, typedecl, &d->front.file, 0, 0, 2, 0,
                                   type_name, op1);

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_create_symbol_tables(d->front.analyzer));

    struct ast_node *td = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(td, "typedecl node was not found");
    st = ast_typedecl_symbol_table_get(td);

    testsupport_symbol_table_lookup_record(st, &id1s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_STRING);
    check_ast_match(symbol_table_record_node(rec), id2, &d->front.file);

    testsupport_symbol_table_lookup_record(st, &id2s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_UINT_16);
    check_ast_match(symbol_table_record_node(rec), id4, &d->front.file);

    testsupport_symbol_table_lookup_record(st, &names, rec, false);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_USER_DEFINED);
    check_ast_match(symbol_table_record_node(rec), tdecl, &d->front.file);


    ast_node_destroy(tdecl);
} END_TEST

START_TEST(test_multiple_level_symbol_tables) {
    struct symbol_table *st;
    struct symbol_table_record *rec;
    static const struct RFstring names = RF_STRING_STATIC_INIT("person");
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("name");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("age");
    static const struct RFstring v1s = RF_STRING_STATIC_INIT("var1");
    static const struct RFstring v2s = RF_STRING_STATIC_INIT("var2");
    static const struct RFstring v3s = RF_STRING_STATIC_INIT("var3");
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "var1:string\n"
        "{\n"
        "type person {\n"
        "name:string, age:u16\n"
        "}\n"
        "var2:i8\n"
        "}\n"
        "var3:u64"
    );
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    struct ast_node *type_name = testsupport_parser_identifier_create(
        &d->front.file,
        2, 5, 2, 10);
    struct ast_node *id1 = testsupport_parser_identifier_create(
        &d->front.file,
        3, 0, 3, 3);
    testsupport_parser_xidentifier_create_simple(id2, &d->front.file,
                                                 3, 5, 3, 10);
    testsupport_parser_node_create(t1, typedesc, &d->front.file,
                                   3, 0, 3, 10, id1, id2);


    struct ast_node *id3 = testsupport_parser_identifier_create(
        &d->front.file,
        3, 13, 3, 15);
    testsupport_parser_xidentifier_create_simple(id4, &d->front.file,
                                                 3, 17, 3, 19);
    testsupport_parser_node_create(t2, typedesc, &d->front.file,
                                   3, 13, 3, 19, id3, id4);

    testsupport_parser_node_create(op1, typeop, &d->front.file, 3, 0, 3, 19,
                                   TYPEOP_PRODUCT, t1, t2);
    testsupport_parser_node_create(tdecl, typedecl, &d->front.file, 2, 0, 4, 0,
                                   type_name, op1);


    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_create_symbol_tables(d->front.analyzer));

    struct ast_node *root = d->front.analyzer->root;
    struct ast_node *block_1 = ast_node_get_child(root, 1);
    struct ast_node *td = ast_node_get_child(block_1, 0);
    ck_assert_msg(td, "typedecl node was not found");

    // first check for symbols at root
    st = ast_root_symbol_table_get(root);
    testsupport_symbol_table_lookup_record(st, &v1s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_STRING);
    testsupport_symbol_table_lookup_record(st, &v3s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_UINT_64);

    // now check for symbols at the block
    st = ast_block_symbol_table_get(block_1);
    testsupport_symbol_table_lookup_record(st, &names, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_USER_DEFINED);
    check_ast_match(symbol_table_record_node(rec), tdecl, &d->front.file);
    testsupport_symbol_table_lookup_record(st, &v2s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_INT_8);

    // finally check for symbols inside the type declaration
    st = ast_typedecl_symbol_table_get(td);
    testsupport_symbol_table_lookup_record(st, &id1s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_STRING);
    check_ast_match(symbol_table_record_node(rec), id2, &d->front.file);

    testsupport_symbol_table_lookup_record(st, &id2s, rec, true);
    ck_assert(symbol_table_record_category(rec) == TYPE_CATEGORY_BUILTIN);
    ck_assert(type_builtin(symbol_table_record_type(rec)) == BUILTIN_UINT_16);
    check_ast_match(symbol_table_record_node(rec), id4, &d->front.file);


    ast_node_destroy(tdecl);
} END_TEST

Suite *analyzer_symboltable_suite_create(void)
{
    Suite *s = suite_create("analyzer_symbol_table");

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
    tcase_add_test(st2, test_fndecl_symbol_table);
    tcase_add_test(st2, test_typedecl_symbol_table);
    tcase_add_test(st2, test_multiple_level_symbol_tables);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    return s;
}


