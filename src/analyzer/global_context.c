#include "global_context.h"

#include <ast/ast.h>
#include <analyzer/analyzer.h>
#include <types/type.h>
#include <types/type_elementary.h>

bool analyzer_load_globals(struct analyzer *a)
{
    //add print() to special functions
    static const struct RFstring arg_name1 = RF_STRING_STATIC_INIT("a");
    static const struct RFstring arg_name2 = RF_STRING_STATIC_INIT("b");

    static const struct RFstring print_fn_name = RF_STRING_STATIC_INIT("print");
    struct symbol_table *root_st = ast_root_symbol_table_get(a->root);
    struct type *type_string = type_leaf_create(
        a,
        &arg_name1,
        type_elementary_get_type_constant(ELEMENTARY_TYPE_STRING));
    if (!type_string) {
        goto fail;
    }
    struct type *type_int = type_leaf_create(
        a,
        &arg_name2,
        type_elementary_get_type_constant(ELEMENTARY_TYPE_INT_64)
    );
    if (!type_int) {
        goto fail;
    }

    struct type *arg_type = type_operator_create(a, type_string, type_int, TYPEOP_SUM);
    if (!arg_type) {
        goto fail;
    }

    struct type *print_fn_type = type_function_create(
        a,
        arg_type,
        type_elementary_get_type(ELEMENTARY_TYPE_NIL));
    if (!print_fn_type) {
        return false;
    }
    return symbol_table_add_type(root_st, a, &print_fn_name, print_fn_type);

fail:
    RF_ERROR("Failed to create print() function type");
    return false;
}
