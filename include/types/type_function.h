#ifndef LFR_TYPES_FUNCTION_H
#define LFR_TYPES_FUNCTION_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

i_INLINE_DECL bool type_is_function(const struct type *t)
{
    return t->category == TYPE_CATEGORY_COMPOSITE &&
           t->composite.is_operator &&
           t->composite.op.type == TYPEOP_IMPLICATION;
}

//! Gets the type descriptions of the arguments of the function
i_INLINE_DECL struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    return t->composite.op.left;
}

i_INLINE_DECL void type_function_set_argtype(struct type *t, struct type *other)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    t->composite.op.left = other;
}

i_INLINE_DECL struct type *type_function_get_rettype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    return t->composite.op.right;
}

i_INLINE_DECL void type_function_set_rettype(struct type *t, struct type *other)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    t->composite.op.right = other;
}

/**
 * Initialize a type structure as a function type
 */
void type_function_init(struct type *t, struct type *arg_type, struct type *ret_type);

/** @return the type of a particular argument or NULL if there was a
 * malformed expression type or @n is out of bounds
 */
const struct type *type_function_get_argtype_n(const struct type *t, unsigned int n);

#endif
