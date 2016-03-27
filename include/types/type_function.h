#ifndef LFR_TYPES_FUNCTION_H
#define LFR_TYPES_FUNCTION_H

#include <rflib/defs/inline.h>
#include <rflib/utils/sanity.h>

#include <types/type_decls.h>
#include <types/type_elementary.h>

struct analyzer;
struct module;

i_INLINE_DECL bool type_is_function(const struct type *t)
{
    return (t->category == TYPE_CATEGORY_OPERATOR &&
            t->operator.type == TYPEOP_IMPLICATION);
}

/**
 * Returns true if the type can be called.
 *
 * Callable types are functions, custom user defined types since
 * they can have a constructor and almost all elementary types for
 * explicit conversion
 */
i_INLINE_DECL bool type_is_callable(const struct type *t)
{
    return type_is_function(t) ||
        t->category == TYPE_CATEGORY_DEFINED ||
        type_is_explicitly_convertable_elementary(t);
}

struct type *type_function_get_argtype(const struct type *t);
struct type *type_function_get_rettype(const struct type *t);

//! Gets the type description of the arguments of a callable type
i_INLINE_DECL const struct type *type_callable_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_callable(t) && !type_is_explicitly_convertable_elementary(t),
              "Function should be called for callable types and not conversions");
    if (type_is_function(t)) {
        return type_function_get_argtype(t);
    }
    // else it's a constructor of a defined type
    return t;
}

i_INLINE_DECL struct type *type_callable_get_rettype(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return type_function_get_rettype(t);
    }
    // else it's either a constructor of a defined type or explicit conversion,
    // so returned type is actually t
    return (struct type*)t;
}

/**
 * Provided a callable type will return a pointer to a string of either
 * "function" or "constructor"
 */
const struct RFstring *type_callable_category_str(const struct type *t);

/**
 * Initialize a type structure as a function type
 */
void type_function_init(
    struct type *t,
    struct type *arg_type,
    struct type *ret_type
);
struct type *type_function_create(
    struct module *m,
    struct type *arg_type,
    struct type *ret_type
);

#endif
