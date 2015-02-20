#ifndef LFR_TYPES_FUNCTION_H
#define LFR_TYPES_FUNCTION_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

i_INLINE_DECL bool type_is_function(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR &&
           t->operator.type == TYPEOP_IMPLICATION;
}

/**
 * Returns true if the type can be called.
 *
 * Callable types are functions and also custom user defined types since
 * they can have a constructor
 */
i_INLINE_DECL bool type_is_callable(const struct type *t)
{
    return type_is_function(t) || t->category == TYPE_CATEGORY_DEFINED;
}

//! Gets the type descriptions of the arguments of the function
i_INLINE_DECL struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    return t->operator.left;
}

//! Gets the type description of the arguments of a callable type
i_INLINE_DECL struct type *type_callable_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return type_function_get_argtype(t);
    }
    // else it's a constructor of a defined type
    return t->defined.type;
}

i_INLINE_DECL void type_function_set_argtype(struct type *t, struct type *other)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    t->operator.left = other;
}

i_INLINE_DECL struct type *type_function_get_rettype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    return t->operator.right;
}

i_INLINE_DECL struct type *type_callable_get_rettype(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return type_function_get_rettype(t);
    }
    // else it's a constructor of a defined type, so returned type is actually t
    return (struct type*)t;
}

i_INLINE_DECL void type_function_set_rettype(struct type *t, struct type *other)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    t->operator.right = other;
}

/**
 * Provided a callable type will return a pointer to a string of either
 * "function" or "constructor"
 */
const struct RFstring *type_callable_category_str(const struct type *t);

/**
 * Initialize a type structure as a function type
 */
void type_function_init(struct type *t, struct type *arg_type, struct type *ret_type);

/** @return the type of a particular argument or NULL if there was a
 * malformed expression type or @n is out of bounds
 */
const struct type *type_function_get_argtype_n(const struct type *t, unsigned int n);

#endif
