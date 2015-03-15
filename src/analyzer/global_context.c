#include "global_context.h"

#include <ast/ast.h>
#include <analyzer/analyzer.h>
#include <types/type.h>
#include <types/type_elementary.h>

bool analyzer_load_globals(struct analyzer *a)
{
    //add print() to special functions
    static const struct RFstring arg_name = RF_STRING_STATIC_INIT("s");
    static const struct RFstring print_fn_name = RF_STRING_STATIC_INIT("print");
    struct symbol_table *root_st = ast_root_symbol_table_get(a->root);
    struct type *arg_type = type_leaf_create(
        a,
        &arg_name,
        type_elementary_get_type_constant(ELEMENTARY_TYPE_STRING));
    if (!arg_type) {
        return false;
    }
    struct type *print_fn_type = type_function_create(
        a,
        arg_type,
        type_elementary_get_type(ELEMENTARY_TYPE_NIL));
    if (!print_fn_type) {
        return false;
    }
    return symbol_table_add_type(root_st, a, &print_fn_name, print_fn_type);
}
