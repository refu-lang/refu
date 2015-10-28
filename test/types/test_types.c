#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <ast/type.h>

#include "../testsupport_front.h"
#include "../parser/testsupport_parser.h"
#include "../analyzer/testsupport_analyzer.h"

#include CLIB_TEST_HELPERS

START_TEST (test_type_comparison_identical) {
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);


    // normal type comparison should work
    ck_assert(type_compare(t_i8, t_i8, TYPECMP_IMPLICIT_CONVERSION));
    ck_assert(type_compare(t_string, t_string, TYPECMP_IMPLICIT_CONVERSION));

    // identical types should also work
    ck_assert(type_compare(t_i8, t_i8, TYPECMP_IDENTICAL));
    ck_assert(type_compare(t_string, t_string, TYPECMP_IDENTICAL));

    ck_assert(!type_compare(t_i8, t_i64, TYPECMP_IDENTICAL));

} END_TEST

START_TEST (test_type_comparison_for_sum_fncall) {
    // test for a bug concerning subtypes of sum types
    // make type a:i64 | b:u64 | c:f64 | d:string
    // just like the typechecking for function calls
    // check that comparing a subtype gives the correct matched type

    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_i64,
                                                                   t_u64,
                                                                   t_f64,
                                                                   t_string);

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_u64, t_sum, TYPECMP_PATTERN_MATCHING));
    const struct type *matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_u64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_i64, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_i64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_f64, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_f64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_string, t_sum, TYPECMP_PATTERN_MATCHING));
    matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_string, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));

} END_TEST

START_TEST (test_type_comparison_for_sum_fncall_with_conversion) {
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);

    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_i64,
                                                                   t_f64,
                                                                   t_string);

    typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
    ck_assert(type_compare(t_i8, t_sum, TYPECMP_PATTERN_MATCHING));
    const struct type *matched_type = typemp_ctx_get_matched_type();
    ck_assert_msg(matched_type == t_i64, "Unexpected match type "RF_STR_PF_FMT" found", RF_STR_PF_ARG(type_str_or_die(matched_type, TSTR_DEFAULT)));
} END_TEST

START_TEST (test_elementary_get_category) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert_int_eq(type_elementary_get_category(t_i), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i8), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u8), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i16), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u16), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i32), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u32), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_i64), ELEMENTARY_TYPE_CATEGORY_SIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_u64), ELEMENTARY_TYPE_CATEGORY_UNSIGNED);
    ck_assert_int_eq(type_elementary_get_category(t_f32), ELEMENTARY_TYPE_CATEGORY_FLOAT);
    ck_assert_int_eq(type_elementary_get_category(t_f64), ELEMENTARY_TYPE_CATEGORY_FLOAT);
    ck_assert_int_eq(type_elementary_get_category(t_string), ELEMENTARY_TYPE_CATEGORY_OTHER);
    ck_assert_int_eq(type_elementary_get_category(t_bool), ELEMENTARY_TYPE_CATEGORY_OTHER);
    ck_assert_int_eq(type_elementary_get_category(t_nil), ELEMENTARY_TYPE_CATEGORY_OTHER);
} END_TEST

START_TEST (test_is_signed_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(type_is_signed_elementary(t_i));
    ck_assert(!type_is_signed_elementary(t_u));
    ck_assert(type_is_signed_elementary(t_i8));
    ck_assert(!type_is_signed_elementary(t_u8));
    ck_assert(type_is_signed_elementary(t_i16));
    ck_assert(!type_is_signed_elementary(t_u16));
    ck_assert(type_is_signed_elementary(t_i32));
    ck_assert(!type_is_signed_elementary(t_u32));
    ck_assert(type_is_signed_elementary(t_i64));
    ck_assert(!type_is_signed_elementary(t_u64));
    ck_assert(!type_is_signed_elementary(t_f32));
    ck_assert(!type_is_signed_elementary(t_f64));
    ck_assert(!type_is_signed_elementary(t_string));
    ck_assert(!type_is_signed_elementary(t_bool));
    ck_assert(!type_is_signed_elementary(t_nil));
} END_TEST

START_TEST (test_is_unsigned_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(!type_is_unsigned_elementary(t_i));
    ck_assert(type_is_unsigned_elementary(t_u));
    ck_assert(!type_is_unsigned_elementary(t_i8));
    ck_assert(type_is_unsigned_elementary(t_u8));
    ck_assert(!type_is_unsigned_elementary(t_i16));
    ck_assert(type_is_unsigned_elementary(t_u16));
    ck_assert(!type_is_unsigned_elementary(t_i32));
    ck_assert(type_is_unsigned_elementary(t_u32));
    ck_assert(!type_is_unsigned_elementary(t_i64));
    ck_assert(type_is_unsigned_elementary(t_u64));
    ck_assert(!type_is_unsigned_elementary(t_f32));
    ck_assert(!type_is_unsigned_elementary(t_f64));
    ck_assert(!type_is_unsigned_elementary(t_string));
    ck_assert(!type_is_unsigned_elementary(t_bool));
    ck_assert(!type_is_unsigned_elementary(t_nil));
} END_TEST

START_TEST (test_is_floating_elementary) {

    struct type *t_i = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT, false);
    struct type *t_u = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT, false);
    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_i16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_16, false);
    struct type *t_u16 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_16, false);
    struct type *t_i32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_32, false);
    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_bool = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_BOOL, false);
    struct type *t_nil = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_NIL, false);

    ck_assert(!type_is_floating_elementary(t_i));
    ck_assert(!type_is_floating_elementary(t_u));
    ck_assert(!type_is_floating_elementary(t_i8));
    ck_assert(!type_is_floating_elementary(t_u8));
    ck_assert(!type_is_floating_elementary(t_i16));
    ck_assert(!type_is_floating_elementary(t_u16));
    ck_assert(!type_is_floating_elementary(t_i32));
    ck_assert(!type_is_floating_elementary(t_u32));
    ck_assert(!type_is_floating_elementary(t_i64));
    ck_assert(!type_is_floating_elementary(t_u64));
    ck_assert(type_is_floating_elementary(t_f32));
    ck_assert(type_is_floating_elementary(t_f64));
    ck_assert(!type_is_floating_elementary(t_string));
    ck_assert(!type_is_floating_elementary(t_bool));
    ck_assert(!type_is_floating_elementary(t_nil));
} END_TEST

START_TEST(test_determine_block_type1) {
    struct ast_node *block;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "{\n"
        "d:f64 = 3.14 * 0.14\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    block = ast_node_get_child(front_testdriver_module()->node, 0);
    ck_assert_msg(block, "Block should be the first child of the root");
    const struct type *block_type = ast_node_get_type(block);
    ck_assert_msg(block_type, "Block should have a type");
    ck_assert_msg(type_compare(block_type, t_f64, TYPECMP_IDENTICAL),
                  "Expected the block's type to be an f64");
} END_TEST

START_TEST(test_determine_block_type2) {
    struct ast_node *block;
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string}\n"
        "{\n"
        "a:foo\n"
        "}"
    );
    front_testdriver_new_main_source(&s);
    ck_assert_typecheck_ok();

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_i8,
                                                                      t_string);
    static const struct RFstring id_foo = RF_STRING_STATIC_INIT("foo");
    struct type *t_foo = testsupport_analyzer_type_create_defined(&id_foo, t_prod_1);

    block = ast_node_get_child(front_testdriver_module()->node, 1);
    ck_assert_msg(block, "Block should be the second child of the root");
    const struct type *block_type = ast_node_get_type(block);
    ck_assert_msg(block_type, "Block should have a type");
    ck_assert_msg(type_compare(block_type, t_foo, TYPECMP_IDENTICAL),
                  "Expected the block's type to be an f64");
} END_TEST


struct test_traversal_cb_ctx {
    unsigned int idx;
    const struct RFstring *names;
    struct type **types;
};

static void test_traversal_cb_ctx_init(
    struct test_traversal_cb_ctx *ctx,
    const struct RFstring *names,
    struct type **types)
{
    ctx->idx = 0;
    ctx->names = names;
    ctx->types = types;
}

static bool test_traversal_cb(
    const struct RFstring *name,
    const struct ast_node *desc,
    struct type *t,
    struct test_traversal_cb_ctx *ctx)
{
    ck_assert_msg(
        rf_string_equal(&ctx->names[ctx->idx], name),
        "Ast type traversal index %u expected name "RF_STR_PF_FMT" but got "
        RF_STR_PF_FMT".",
        ctx->idx,
        RF_STR_PF_ARG(&ctx->names[ctx->idx]),
        RF_STR_PF_ARG(name)
    );
    ck_assert_msg(
        type_compare(ctx->types[ctx->idx], t, TYPECMP_IDENTICAL),
        "Ast type traversal index %u expected type "RF_STR_PF_FMT" but got "
        RF_STR_PF_FMT".",
        ctx->idx,
        RF_STR_PF_ARG(type_str(ctx->types[ctx->idx], TSTR_DEFAULT)),
        RF_STR_PF_ARG(type_str(t, TSTR_DEFAULT))
    );
    ++ctx->idx;
    return true;
}

START_TEST(test_type_ast_traversal1) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string}\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_prod = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_i8,
                                                                      t_string);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b")
    };
    struct type *expected_types [] = { t_i8, t_string };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_prod, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST(test_type_ast_traversal2) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string, c:f32}\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_prod = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                      t_i8,
                                                                      t_string,
                                                                      t_f32);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("c"),
    };
    struct type *expected_types [] = { t_i8, t_string, t_f32 };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_prod, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST(test_type_ast_traversal3) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string, c:f32, d:u64}\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_prod = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                    t_i8,
                                                                    t_string,
                                                                    t_f32,
                                                                    t_u64);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("c"),
        RF_STRING_STATIC_INIT("d"),
    };
    struct type *expected_types [] = { t_i8, t_string, t_f32, t_u64 };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_prod, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST(test_type_ast_traversal4) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string | c:f32, d:u64}\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_prod1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_i8,
                                                                     t_string);
    struct type *t_prod2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_f32,
                                                                     t_u64);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_prod1,
                                                                   t_prod2);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("c"),
        RF_STRING_STATIC_INIT("d"),
    };
    struct type *expected_types [] = { t_i8, t_string, t_f32, t_u64 };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_sum, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST(test_type_ast_traversal5) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string, c:i64 | d:f32, e:u64, f:string, g:u8}\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_prod1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_i8,
                                                                     t_string,
                                                                     t_i64);
    struct type *t_prod2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_f32,
                                                                     t_u64,
                                                                     t_string,
                                                                     t_u8);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_prod1,
                                                                   t_prod2);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("c"),
        RF_STRING_STATIC_INIT("d"),
        RF_STRING_STATIC_INIT("e"),
        RF_STRING_STATIC_INIT("f"),
        RF_STRING_STATIC_INIT("g"),
    };
    struct type *expected_types [] = { t_i8, t_string, t_i64,
                                       t_f32, t_u64, t_string, t_u8 };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_sum, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST(test_type_ast_traversal6) {
    static const struct RFstring s = RF_STRING_STATIC_INIT(
        "type foo {a:i8, b:string, c:i64 | d:f32, e:u64, f:string, g:u8 | h:i8 | z:string }\n"
    );
    front_testdriver_new_main_source(&s);
    testsupport_scan_and_parse(); // go only up to the parsing stage, don't analyze

    struct type *t_i8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_8, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_i64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_INT_64, false);
    struct type *t_f32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_32, false);
    struct type *t_u64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_64, false);
    struct type *t_u8 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_8, false);
    struct type *t_prod1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_i8,
                                                                     t_string,
                                                                     t_i64);
    struct type *t_prod2 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_f32,
                                                                     t_u64,
                                                                     t_string,
                                                                     t_u8);
    struct type *t_sum = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                   t_prod1,
                                                                   t_prod2,
                                                                   t_i8,
                                                                   t_string);

    struct ast_node *ast_desc = ast_typedecl_typedesc_get(ast_node_get_child(front_testdriver_module()->node, 0));
    struct test_traversal_cb_ctx ctx;
    const struct RFstring expected_names[] = {
        RF_STRING_STATIC_INIT("a"),
        RF_STRING_STATIC_INIT("b"),
        RF_STRING_STATIC_INIT("c"),
        RF_STRING_STATIC_INIT("d"),
        RF_STRING_STATIC_INIT("e"),
        RF_STRING_STATIC_INIT("f"),
        RF_STRING_STATIC_INIT("g"),
        RF_STRING_STATIC_INIT("h"),
        RF_STRING_STATIC_INIT("z"),
    };
    struct type *expected_types [] = {
        t_i8, t_string, t_i64,
        t_f32, t_u64, t_string, t_u8,
        t_i8,
        t_string
    };
    test_traversal_cb_ctx_init(&ctx, expected_names, expected_types);
    ck_assert(ast_type_foreach_arg(ast_desc, t_sum, (ast_type_cb)test_traversal_cb, &ctx));
} END_TEST

START_TEST (test_type_to_str) {

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    struct type *t_string = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_STRING, false);
    struct type *t_prod_1 = testsupport_analyzer_type_create_operator(TYPEOP_PRODUCT,
                                                                     t_u32,
                                                                     t_f64);
    struct type *t_sum_1 = testsupport_analyzer_type_create_operator(TYPEOP_SUM,
                                                                     t_f64,
                                                                     t_string);
    static const struct RFstring id_person =  RF_STRING_STATIC_INIT("person");
    struct type *t_defined_1 = testsupport_analyzer_type_create_defined(&id_person,
                                                                        t_sum_1);

    struct RFstring *ts;
    RFS_PUSH();
    ck_assert((ts = type_str(t_u32, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "u32");
    ck_assert((ts = type_str(t_prod_1, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "u32,f64");
    ck_assert((ts = type_str(t_defined_1, TSTR_DEFAULT)));
    ck_assert_rf_str_eq_cstr(ts, "person");
    RFS_POP();
} END_TEST

START_TEST (test_type_op_create_str) {

    struct type *t_u32 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_UINT_32, false);
    struct type *t_f64 = testsupport_analyzer_type_create_elementary(ELEMENTARY_TYPE_FLOAT_64, false);
    RFS_PUSH();
    ck_assert_rf_str_eq_cstr(type_op_create_str(t_u32, t_f64, TYPEOP_PRODUCT), "u32,f64");
    ck_assert_rf_str_eq_cstr(type_op_create_str(t_u32, t_f64, TYPEOP_SUM), "u32|f64");
    ck_assert_rf_str_eq_cstr(type_op_create_str(t_u32, t_f64, TYPEOP_IMPLICATION), "u32->f64");
    RFS_POP();
} END_TEST


Suite *types_suite_create(void)
{
    Suite *s = suite_create("types");

    TCase *st1 = tcase_create("types_comparison_tests");
    tcase_add_checked_fixture(st1, setup_analyzer_tests_no_source, teardown_analyzer_tests);
    tcase_add_test(st1, test_type_comparison_identical);
    tcase_add_test(st1, test_type_comparison_for_sum_fncall);
    tcase_add_test(st1, test_type_comparison_for_sum_fncall_with_conversion);

    TCase *st2 = tcase_create("types_getter_tests");
    tcase_add_checked_fixture(st2, setup_analyzer_tests_no_source, teardown_analyzer_tests);
    tcase_add_test(st2, test_elementary_get_category);
    tcase_add_test(st2, test_is_signed_elementary);
    tcase_add_test(st2, test_is_unsigned_elementary);
    tcase_add_test(st2, test_is_floating_elementary);

    TCase *st3 = tcase_create("type_determination");
    tcase_add_checked_fixture(st3, setup_analyzer_tests, teardown_analyzer_tests);
    tcase_add_test(st3, test_determine_block_type1);
    tcase_add_test(st3, test_determine_block_type2);

    TCase *st4 = tcase_create("type_ast_traversal");
    tcase_add_checked_fixture(st4, setup_analyzer_tests_no_stdlib, teardown_analyzer_tests);
    tcase_add_test(st4, test_type_ast_traversal1);
    tcase_add_test(st4, test_type_ast_traversal2);
    tcase_add_test(st4, test_type_ast_traversal3);
    tcase_add_test(st4, test_type_ast_traversal4);
    tcase_add_test(st4, test_type_ast_traversal5);
    tcase_add_test(st4, test_type_ast_traversal6);

    TCase *st5 = tcase_create("type_strings");
    tcase_add_checked_fixture(st5, setup_analyzer_tests_no_source, teardown_analyzer_tests);
    tcase_add_test(st5, test_type_to_str);
    tcase_add_test(st5, test_type_op_create_str);

    suite_add_tcase(s, st1);
    suite_add_tcase(s, st2);
    suite_add_tcase(s, st3);
    suite_add_tcase(s, st4);
    suite_add_tcase(s, st5);
    return s;
}
