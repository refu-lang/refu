#ifndef LFR_TYPES_TYPE_ARRAY_H
#define LFR_TYPES_TYPE_ARRAY_H

#include <rflib/string/common.h>
#include <rflib/defs/inline.h>
#include <types/type_decls.h>

struct module;

void type_arr_init(struct type_arr *arr, struct arr_int64 *dimensions);
struct type_arr *type_arr_create(struct arr_int64 *dimensions);
struct type_arr *type_arr_create_from_ast(struct ast_node *astarr);
void type_arr_destroy(struct type_arr *arr);

bool type_arr_equal(const struct type_arr *a1, const struct type_arr *a2);

/**
 * Returns a string representation of the array specifier
 *
 * Should be enclosed in @ref RFS_PUSH() and @ref RFS_POP().
 */
const struct RFstring* type_arr_str(const struct type_arr *arr);

/**
 * Convenience function to add an array specifier string to a type
 *
 * Should be enclosed in @ref RFS_PUSH() and @ref RFS_POP().
 */
i_INLINE_DECL struct RFstring *type_str_add_array(
    struct RFstring *str,
    const struct type_arr *arr)
{
    return RFS(RFS_PF RFS_PF, RFS_PA(str), RFS_PA(type_arr_str(arr)));
}

struct type *module_getorcreate_type_as_arr(
    struct module *mod,
    const struct type *t,
    struct type_arr *arrtype
);

struct type *module_getorcreate_type_as_singlearr(
    struct module *mod,
    const struct type *t,
    int64_t dimensions
);
#endif
