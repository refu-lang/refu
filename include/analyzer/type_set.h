#ifndef LFR_TYPES_SET_H
#define LFR_TYPES_SET_H

#include <Data_Structures/objset.h>
#include <Definitions/inline.h>

struct type;
struct rir_type;

i_INLINE_DECL const void *type_objset_key(const struct type *t)
{
    return (const void*)t;
}

size_t type_objset_hashfn(const struct type *t);
bool type_objset_eqfn(const struct type *t1,
                      const struct type *t2);

OBJSET_DEFINE_TYPE(type,
                   struct type,
                   type_objset_key,
                   type_objset_hashfn,
                   type_objset_eqfn)

/**
 * Check if a type can be converted to any type in the set
 *
 * @param set        The type set in question
 * @param type       The type to check if can be converted to any other type
 *                   in the set
 * @return           True if @a type can be converted to a type in the set and
 *                   false otherwise
 */
bool type_objset_has_convertable(const struct rf_objset_type *set,
                                 const struct type *type);

#endif
