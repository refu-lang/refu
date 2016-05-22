#ifndef LFR_STRING_SET_H
#define LFR_STRING_SET_H

#include <rfbase/datastructs/objset.h>
#include <rfbase/defs/inline.h>

struct RFstring;

i_INLINE_DECL const void *string_objset_key(const struct RFstring *s)
{
    return (const void*)s;
}

size_t string_objset_hashfn(const struct RFstring *s);
bool string_objset_eqfn(const struct RFstring *s1,
                        const struct RFstring *s2);

OBJSET_DEFINE_TYPE(string,
                   struct RFstring,
                   string_objset_key,
                   string_objset_hashfn,
                   string_objset_eqfn);

#endif
