#ifndef LFR_TYPES_ELEMENTARY_H
#define LFR_TYPES_ELEMENTARY_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

struct type_comparison_ctx;

enum elementary_type_category {
    ELEMENTARY_TYPE_CATEGORY_SIGNED = 0,
    ELEMENTARY_TYPE_CATEGORY_UNSIGNED,
    ELEMENTARY_TYPE_CATEGORY_FLOAT,
    ELEMENTARY_TYPE_CATEGORY_OTHER,
};

bool type_elementary_equals(const struct type_elementary *t1,
                            const struct type_elementary *t2,
                            struct type_comparison_ctx *ctx);

/**
 * Given a built-in type value, returns the type itself
 */
struct type *type_elementary_get_type(enum elementary_type etype);

/**
 * Given a built-in type value return the type string representation
 */
const struct RFstring *type_elementary_get_str(enum elementary_type etype);

/**
 * Check if @c id is an elementary identifier
 */
int type_elementary_identifier_p(const struct RFstring *id);

/**
 * Get the general elementary type category of the given elementary type
 * @param t     The elementary type to query. Must be an elementary type.
 * @return      For possible values @look enum elementary_type_category. If t
 *              is not an elementary type -1 is returned.
 */
enum elementary_type_category type_elementary_get_category(const struct type *t);

/**
 * Given a type, check if it's a specific elementary type
 */
i_INLINE_DECL bool type_is_specific_elementary(const struct type *t, enum elementary_type etype)
{
    return t->category == TYPE_CATEGORY_ELEMENTARY && t->elementary.etype == etype;
}

/**
 * Given a type check if it's any elementary type except string
 */
i_INLINE_DECL bool type_is_simple_elementary(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ELEMENTARY && t->elementary.etype != ELEMENTARY_TYPE_STRING;
}

/**
 * Gets the elementary type of a type provided it is indeed an elementary type
 */
i_INLINE_DECL enum elementary_type type_elementary(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_ELEMENTARY,
              "Non built-in type category detected");
    return t->elementary.etype;
}

/**
 * Check if the type is a signed elementary type
 */
i_INLINE_DECL bool type_is_signed_elementary(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ELEMENTARY &&
        t->elementary.etype < 10 &&
        t->elementary.etype % 2 == 0;
}

/**
 * Check if the type is an unsigned elementary type
 */
i_INLINE_DECL bool type_is_unsigned_elementary(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ELEMENTARY &&
        t->elementary.etype < 10 &&
        t->elementary.etype % 2 == 1;
}

/**
 * Check if the type is an floating elementary type
 */
i_INLINE_DECL bool type_is_floating_elementary(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ELEMENTARY &&
        (t->elementary.etype == ELEMENTARY_TYPE_FLOAT_32 || 
         t->elementary.etype == ELEMENTARY_TYPE_FLOAT_64);
}
#endif
