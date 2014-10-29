#include <analyzer/typecheck.h>

#include <Utils/build_assert.h>

#include <ast/ast.h>
#include <analyzer/symbol_table.h>

#include "builtin_types_htable.h"

static const struct RFstring builtin_type_strings[] = {
    RF_STRING_STATIC_INIT("u8"),
    RF_STRING_STATIC_INIT("i8"),
    RF_STRING_STATIC_INIT("u16"),
    RF_STRING_STATIC_INIT("i16"),
    RF_STRING_STATIC_INIT("u32"),
    RF_STRING_STATIC_INIT("i32"),
    RF_STRING_STATIC_INIT("u64"),
    RF_STRING_STATIC_INIT("i64"),
    RF_STRING_STATIC_INIT("f32"),
    RF_STRING_STATIC_INIT("f64"),
    RF_STRING_STATIC_INIT("string"),
};


void type_builtin_init(struct type *t, enum builtin_type bt)
{
    RF_ASSERT(bt >= 0, "illegal builtin data type given");

    t->category = TYPE_CATEGORY_BUILTIN;
    t->btype = bt;
}

void type_init(struct type *t, enum type_category ctg, struct ast_node *decl)
{
    t->category = ctg;
    t->decl = decl;
}

i_INLINE_INS enum builtin_type type_builtin(struct type *t);

int analyzer_identifier_is_builtin(const struct RFstring *id)
{
    const struct gperf_builtin_type *btype;
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(builtin_type_strings)/sizeof(struct RFstring) == BUILTIN_TYPES_COUNT
    );

    btype = analyzer_string_is_builtin(rf_string_data(id),
                                       rf_string_length_bytes(id));

    if (!btype) {
        return -1;
    }

    return btype->type;
}

static bool analyzer_type_check_do(struct ast_node *typenode,
                                   struct symbol_table *st)
{
    switch(typenode->type) {
    case AST_TYPE_DESCRIPTION:
        if (typenode->typedesc.left) {
            analyzer_type_check_do(typenode->typedesc.left, st);

        }
        if (typenode->typedesc.right) {
            analyzer_type_check_do(typenode->typedesc.right, st);
        }
        break;
    case AST_TYPE_OPERATOR:
        if (typenode->typeop.left) {
            analyzer_type_check_do(typenode->typeop.left, st);
        }
        if (typenode->typeop.right) {
            analyzer_type_check_do(typenode->typeop.right, st);
        }
        break;
    case AST_XIDENTIFIER:
        // TODO
        break;
    case AST_IDENTIFIER:
        // TODO
        break;
    default:
        RF_ASSERT(false, "Unexpected ast node \""RF_STR_PF_FMT"\" at type check",
                  RF_STR_PF_ARG(ast_node_str(typenode)));
        return false;
    }

    return true;
}

bool analyzer_type_check(struct ast_node *typenode, struct symbol_table *st)
{
    return analyzer_type_check_do(typenode, st);
}
