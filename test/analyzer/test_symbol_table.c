#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include <analyzer/types.h>
#include <ast/ast.h>
#include <ast/block.h>
#include <ast/identifier.h>
#include <ast/function.h>
#include <ast/vardecl.h>
#include <ast/type.h>

#include "../../src/analyzer/analyzer_pass1.h"

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

/* -- simple symbol table functionality tests -- */

START_TEST(test_symbol_table_add) {
    struct symbol_table st;
    struct ast_node *n;
    bool at_first;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:i32\n"
        "var_2:string"
    );
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("var_2");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(symbol_table_init(&st, d->front.analyzer));

    struct ast_node *id1 = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                                "a");
    front_testdriver_generate_node(tid1, d, 0, 2, 0, 4,
                                   AST_XIDENTIFIER, 1, "i32");
    front_testdriver_generate_node(t1, d, 0, 0, 0, 4,
                                   AST_TYPE_DESCRIPTION, 2, id1, tid1);
    front_testdriver_generate_node(v1, d, 0, 0, 0, 4,
                                   AST_VARIABLE_DECLARATION, 1, t1);


    struct ast_node *id2 = front_testdriver_generate_identifier(d, 1, 0, 1, 4,
                                                                "var_2");
    front_testdriver_generate_node(tid2, d, 1, 6, 1, 11,
                                   AST_XIDENTIFIER, 1, "string");
    front_testdriver_generate_node(t2, d, 1, 0, 1, 11,
                                   AST_TYPE_DESCRIPTION, 2, id2, tid2);
    front_testdriver_generate_node(v2, d, 1, 0, 1, 11,
                                   AST_VARIABLE_DECLARATION, 1, t2);

    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id1), v1);
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id2), v2);

    n = symbol_table_lookup_node(&st, &id1s, &at_first);
    ck_assert(n == v1);
    ck_assert(at_first);
    n = symbol_table_lookup_node(&st, &id2s, &at_first);
    ck_assert(n == v2);
    ck_assert(at_first);

    symbol_table_deinit(&st);
}END_TEST

START_TEST(test_symbol_table_add_existing) {
    struct symbol_table st;
    struct ast_node *n;
    bool at_first;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "a:i32\n"
        "var_2:string"
    );
    static const struct RFstring id1s = RF_STRING_STATIC_INIT("a");
    static const struct RFstring id2s = RF_STRING_STATIC_INIT("var_2");
    struct front_testdriver *d = get_front_testdriver();
    front_testdriver_assign(d, &s);

    ck_assert(symbol_table_init(&st, d->front.analyzer));

    struct ast_node *id1 = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                                                "a");
    front_testdriver_generate_node(tid1, d, 0, 2, 0, 4,
                                   AST_XIDENTIFIER, 1, "i32");
    front_testdriver_generate_node(t1, d, 0, 0, 0, 4,
                                   AST_TYPE_DESCRIPTION, 2, id1, tid1);
    front_testdriver_generate_node(v1, d, 0, 0, 0, 4,
                                   AST_VARIABLE_DECLARATION, 1, t1);


    struct ast_node *id2 = front_testdriver_generate_identifier(d, 1, 0, 1, 4,
                                                                "var_2");
    front_testdriver_generate_node(tid2, d, 1, 6, 1, 11,
                                   AST_XIDENTIFIER, 1, "string");
    front_testdriver_generate_node(t2, d, 1, 0, 1, 11,
                                   AST_TYPE_DESCRIPTION, 2, id2, tid2);
    front_testdriver_generate_node(v2, d, 1, 0, 1, 11,
                                   AST_VARIABLE_DECLARATION, 1, t2);

    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id1), v1);
    testsupport_symbol_table_add_node(&st, d, ast_identifier_str(id2), v2);
    ck_assert(!symbol_table_add_node(&st, d->front.analyzer,
                                     ast_identifier_str(id2), v2));

    n = symbol_table_lookup_node(&st, &id1s, &at_first);
    ck_assert(n == v1);
    ck_assert(at_first);
    n = symbol_table_lookup_node(&st, &id2s, &at_first);
    ck_assert(n == v2);
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

static struct ast_node *generate_test_vardecl(struct front_testdriver *d,
                                              unsigned int i)
{
    struct ast_node *id;
    id = front_testdriver_generate_identifier(d, 0, 0, 0, 0,
                                              rf_string_data(&ids_arr[i].s));

    front_testdriver_generate_node(xid, d, 0, 0, 0, 2,
                                   AST_XIDENTIFIER, 1, "u32");
    front_testdriver_generate_node(t, d, 1, 0, 1, 11,
                                   AST_TYPE_DESCRIPTION, 2, id, xid);
    front_testdriver_generate_node(v, d, 1, 0, 1, 11,
                                   AST_VARIABLE_DECLARATION, 1, t);
    return v;
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
        n = generate_test_vardecl(d, i);
        ck_assert_msg(n, "Could not generate a test vardecl");
        ids_arr[i].n = n;
        ck_assert_msg(symbol_table_add_node(&st, d->front.analyzer,
                                            &ids_arr[i].s, n),
                      "Could not add %u/%zu generated vardecl to the symbol table",
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
    struct symbol_table_record *rec;
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

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_first_pass(d->front.analyzer));

    struct type *ti64 = testsupport_analyzer_type_create_builtin(BUILTIN_INT_64);
    struct type *tu32 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_32);

    struct ast_node *block = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(block, "block node was not found");
    st = ast_block_symbol_table_get(block);

    testsupport_symbol_table_lookup_record(st, &id1s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), ti64);
    testsupport_symbol_table_lookup_record(st, &id2s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), tu32);
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


    struct type *tu64 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_64);
    struct type *tstring = testsupport_analyzer_type_create_builtin(BUILTIN_STRING);
    struct type *tu32 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_32);
    struct type *l1 = testsupport_analyzer_type_create_leaf(&id1s, tu64);
    struct type *l2 = testsupport_analyzer_type_create_leaf(&id2s, tstring);
    struct type *op1 = testsupport_analyzer_type_create_operator(
        TYPEOP_PRODUCT, l1, l2);
    struct type *tfn = testsupport_analyzer_type_create_function(op1, tu32);



    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_first_pass(d->front.analyzer));

    struct ast_node *fnimpl = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(fnimpl, "fnimpl node was not found");
    st = ast_fndecl_symbol_table_get(ast_fnimpl_fndecl_get(fnimpl));

    testsupport_symbol_table_lookup_record(st, &id1s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), tu64);

    testsupport_symbol_table_lookup_record(st, &id2s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), tstring);


    testsupport_symbol_table_lookup_record(st, &names, rec, false);
    testsupport_types_equal(symbol_table_record_type(rec), tfn);
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



    struct type *tstring = testsupport_analyzer_type_create_builtin(BUILTIN_STRING);
    struct type *tu16 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_16);
    struct type *l1 = testsupport_analyzer_type_create_leaf(&id1s, tstring);
    struct type *l2 = testsupport_analyzer_type_create_leaf(&id2s, tu16);
    struct type *op1 = testsupport_analyzer_type_create_operator(
        TYPEOP_PRODUCT, l1, l2);
    struct type *person_type = testsupport_analyzer_type_create_defined(
        &names, &op1->anonymous
    );

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_first_pass(d->front.analyzer));

    struct ast_node *td = ast_node_get_child(d->front.analyzer->root, 0);
    ck_assert_msg(td, "typedecl node was not found");
    st = ast_root_symbol_table_get(d->front.analyzer->root);

    testsupport_symbol_table_lookup_record(st, &names, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), person_type);
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


    struct type *tstring = testsupport_analyzer_type_create_builtin(BUILTIN_STRING);
    struct type *tu16 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_16);
    struct type *l1 = testsupport_analyzer_type_create_leaf(&id1s, tstring);
    struct type *l2 = testsupport_analyzer_type_create_leaf(&id2s, tu16);
    struct type *op1 = testsupport_analyzer_type_create_operator(
        TYPEOP_PRODUCT, l1, l2);
    struct type *person_type = testsupport_analyzer_type_create_defined(
        &names, &op1->anonymous
    );
    struct type *ti8 = testsupport_analyzer_type_create_builtin(BUILTIN_INT_8);
    struct type *tu64 = testsupport_analyzer_type_create_builtin(BUILTIN_UINT_64);

    testsupport_analyzer_prepare(d, "Preparing for the analyzer phase.");
    ck_assert(analyzer_first_pass(d->front.analyzer));

    struct ast_node *root = d->front.analyzer->root;
    struct ast_node *block_1 = ast_node_get_child(root, 1);
    struct ast_node *td = ast_node_get_child(block_1, 0);
    ck_assert_msg(td, "typedecl node was not found");

    // first check for symbols at root
    st = ast_root_symbol_table_get(root);
    testsupport_symbol_table_lookup_record(st, &v1s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), tstring);
    testsupport_symbol_table_lookup_record(st, &v3s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), tu64);

    // now check for symbols at the block
    st = ast_block_symbol_table_get(block_1);
    testsupport_symbol_table_lookup_record(st, &names, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), person_type);
    testsupport_symbol_table_lookup_record(st, &v2s, rec, true);
    testsupport_types_equal(symbol_table_record_type(rec), ti8);
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


