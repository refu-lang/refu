#ifndef LFR_TYPES_SET_H
#define LFR_TYPES_SET_H

#include <Data_Structures/objset.h>
#include <Definitions/inline.h>

struct type;

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

#endif
