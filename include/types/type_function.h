#ifndef LFR_TYPES_FUNCTION_H
#define LFR_TYPES_FUNCTION_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

i_INLINE_DECL struct type *type_function_get_argtype(struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_FUNCTION, "Non function type detected");
    return t->function.argument_type;
}

i_INLINE_DECL struct type *type_function_get_rettype(struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_FUNCTION, "Non function type detected");
    return t->function.return_type;
}

#endif
