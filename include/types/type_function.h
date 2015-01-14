#ifndef LFR_TYPES_FUNCTION_H
#define LFR_TYPES_FUNCTION_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

//! Gets the type descriptions of the arguments of the function
i_INLINE_DECL struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_FUNCTION, "Non function type detected");
    return t->function.argument_type;
}

i_INLINE_DECL struct type *type_function_get_rettype(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_FUNCTION, "Non function type detected");
    return t->function.return_type;
}

//! Gets the type description of a particular argument or NULL if @c n is out of bounds
const struct type *type_function_get_argtype_n(const struct type *t, unsigned int n);

#endif
