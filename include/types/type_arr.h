#ifndef LFR_TYPES_TYPE_ARRAY_H
#define LFR_TYPES_TYPE_ARRAY_H

#include <rfbase/string/common.h>
#include <rfbase/defs/inline.h>
#include <types/type_decls.h>

struct module;

void type_array_init(
    struct type *tarr,
    const struct type *member_type,
    struct arr_int64 *dimensions
);

struct type *type_array_get_or_create_from_ast(
    struct module *mod,
    struct ast_node *astarr,
    const struct type *member_type
);

void type_array_destroy(struct type *tarr);

/**
 * Get the member type of this array type
 */
i_INLINE_DECL const struct type * type_array_member_type(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_ARRAY, "Should be type of array");
    return t->array.member_type;
}

/**
 * Get the size of the first array dimension
 */
i_INLINE_DECL int64_t type_get_arr_first_size(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_ARRAY, "Should be type of array");
    return darray_size(t->array.dimensions) == 0
        ? -1
        : darray_item(t->array.dimensions, 0);
}

/**
 * Checks whether the given type is an array of elementary types
 */
i_INLINE_DECL bool type_is_elementary_array(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ARRAY &&
        t->array.member_type->category == TYPE_CATEGORY_ELEMENTARY;
}

/**
 * Returns a string representation of the array specifier
 *
 * Should be enclosed in @ref RFS_PUSH() and @ref RFS_POP().
 */
const struct RFstring* type_array_specifier_str(const struct arr_int64 *dims);

/**
 * Convenience function to add an array specifier string to a type
 *
 * Should be enclosed in @ref RFS_PUSH() and @ref RFS_POP().
 */
i_INLINE_DECL struct RFstring *type_str_add_array(
    struct RFstring *str,
    const struct arr_int64 *dimensions)
{
    return RFS(
        RFS_PF RFS_PF,
        RFS_PA(str),
        RFS_PA(type_array_specifier_str(dimensions))
    );
}

/**
 * Retrieve if existing and create if not the array type of @a t
 *
 * @param mod            The module in which to search for the type
 * @param t              The type whose array equivalent type to create
 * @param dimensions     The dimensions of the array to construct in the
 *                       form of an array containing the size of each dimension.
 *                       Size of -1 means dynamic. @note: If the type already
 *                       exists in the module then the given dimensions array
 *                       is freed.
 * @return               The existing type if found or a newly created type if
 *                       it is not found. Also in case of error returns NULL.
 */
struct type *module_getorcreate_type_as_arr(
    struct module *mod,
    const struct type *t,
    struct arr_int64 *dimensions
);

struct type *module_getorcreate_type_as_singlearr(
    struct module *mod,
    const struct type *t,
    int64_t dimensions
);
#endif
