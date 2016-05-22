#include <utils/string_set.h>

#include <rfbase/string/core.h>

i_INLINE_INS const void *string_objset_key(const struct RFstring *s);

size_t string_objset_hashfn(const struct RFstring *s)
{
    return rf_hash_str_stable(s, 0);
}

bool string_objset_eqfn(const struct RFstring *s1,
                        const struct RFstring *s2)
{
    return rf_string_equal(s1, s2);
}
