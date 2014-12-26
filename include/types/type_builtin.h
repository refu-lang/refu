#ifndef LFR_TYPES_BUILTIN_H
#define LFR_TYPES_BUILTIN_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <types/type_decls.h>

struct type_comparison_ctx;

bool type_builtin_equals(const struct type_builtin *t1,
                         const struct type_builtin *t2,
                         struct type_comparison_ctx *ctx);

/**
 * Given a built-in type value, returns the type itself
 */
const struct type *type_builtin_get_type(enum builtin_type btype);

/**
 * Given a built-in type value return the type string representation
 */
const struct RFstring *type_builtin_get_str(enum builtin_type btype);

/**
 * Check if @c id is a builtin identifier
 */
int type_builtin_identifier_p(const struct RFstring *id);

/**
 * Gets the builtin type of a specific builtin type
 */
i_INLINE_DECL enum builtin_type type_builtin(struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_BUILTIN,
              "Non built-in type category detected");
    return t->builtin.btype;
}

#endif
