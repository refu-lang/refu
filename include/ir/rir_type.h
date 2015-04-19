#ifndef LFR_IR_TYPE_H
#define LFR_IR_TYPE_H

#include <ir/rir_type_decls.h>
#include <Data_Structures/darray.h>
#include <Utils/struct_utils.h>
#include <Utils/sanity.h>
#include <types/type_decls.h>

struct type;
struct RFstring;
struct ast_node;
struct symbol_table;

/**
 * Allocates a new rir_type equivalent of @c input.
 *
 * @param input                The type from which to allocate the rir_type
 * @return                     The allocated type
 */
struct rir_type *rir_type_alloc();

/**
 * Create a new allocated rir_type
 */
struct rir_type *rir_type_create(const struct type *input,
                                 const struct RFstring *name,
                                 struct rir_type **newly_created_type);
/**
 * Initialize a rir_type
 */
bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name,
                   struct rir_type **newly_created_type);
/**
 * Initialize a rir_type without proceeding to initialize its children
 *
 * @return If we need to iterate the children of @c input the function will
 *         return true. Else it will return false
 */
bool rir_type_init_before_iteration(struct rir_type *type,
                                    const struct type *input,
                                    const struct RFstring *name);

void rir_type_dealloc(struct rir_type *t);
void rir_type_destroy(struct rir_type *t);
void rir_type_deinit(struct rir_type *t);

/**
 * Returns the type_category of the rir type if it's an operator.
 * @param t    The rir type in questions
 * @return     The type category of the rir type if it's an operator.
 *             If it's not an operator it returns @c RIR_TYPE_CATEGORY_COUNT
 */
enum rir_type_category rir_type_op_from_rir_type(const struct rir_type *t);
/**
 * Returns the rir type op equivalent of the given normal type
 * @param t     A normal type whose rir type operator equivalent to get.
 *              Must belong to category @c TYPE_CATEGORY_OPERATOR
 * @return      The rir type op equivalent of the given normal type.
 */
enum rir_type_category rir_type_op_from_type(const struct type *t);

/**
 * Equality comparison for two rir types
 */
bool rir_type_equals(struct rir_type *a, struct rir_type *b);

/**
 * Equality comparison for a rir and a normal type and name combinarion
 *
 * @param a          The rir type to compare
 * @param b          The normal type to compare to the rir type
 * @param name       An optional name to check for along with the type @c b. Can be NULL
 * @return           True if they are equal and false otherwise
 */
bool rir_type_equals_type(struct rir_type *a, struct type *b, const struct RFstring *name);

/**
 * Checks if @c t is a subtype of @c other. Only checks for pointer equality
 */
bool rir_type_is_subtype_of_other(struct rir_type *t,
                                  struct rir_type *other);

/**
 * A form of @ref rir_type_equals_type for a specific index
 */
bool rir_type_with_index_equals_type(struct rir_type *r_type, unsigned int *index, struct type *n_type);

/**
 * Returns the name of the nth parameter of a type
 * @param t        The type to query
 * @param n        The index (starting from 0) of the parameter's whose name to get
 * @return         The name as a string or NULL if the parameter is out of bounds
 */
const struct RFstring *rir_type_get_nth_name(struct rir_type *t, unsigned n);
i_INLINE_DECL const struct RFstring *rir_type_get_nth_name_or_die(struct rir_type *t, unsigned n)
{
    const struct RFstring *ret = rir_type_get_nth_name(t, n);
    RF_ASSERT_OR_EXIT(ret, "rir_type_get_nth_name_or_die() index out of bounds");
    return ret;
}
/**
 * Returns the type of the nth parameter of a type
 * @param t        The type to query
 * @param n        The index (starting from 0) of the parameter's whose type to get
 * @return         The type or NULL if the parameter is out of bounds
 */
const struct rir_type *rir_type_get_nth_type(struct rir_type *t, unsigned n);
i_INLINE_DECL const struct rir_type *rir_type_get_nth_type_or_die(struct rir_type *t, unsigned n)
{
    const struct rir_type *ret = rir_type_get_nth_type(t, n);
    RF_ASSERT_OR_EXIT(ret, "rir_type_get_nth_type_or_die() index out of bounds");
    return ret;
}

/**
 * Gets a string representation of the rir_type
 *
 * Before this function you need to use @ref RFS_PUSH() in order
 * to remember the temporary string buffer position and after it you need to
 * pop it with @ref RFS_POP().
 *
 * @param[in] t             The type whose string representation to get
 * @return                  Returns the string representation of the rir type
 *                          or NULL if there was an error
 */
struct RFstring *rir_type_str(const struct rir_type *t);
i_INLINE_DECL struct RFstring *rir_type_str_or_die(const struct rir_type *t)
{
    struct RFstring *ret = rir_type_str(t);
    if (!ret) {
        RF_CRITICAL("rir_type_str() failure");
        exit(1);
    }
    return ret;
}


i_INLINE_DECL bool rir_type_is_elementary(const struct rir_type *t)
{
    return t->category < COMPOSITE_PRODUCT_RIR_TYPE;
}
#endif
