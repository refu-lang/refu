#ifndef LFR_IR_RIR_GLOBAL_H
#define LFR_IR_RIR_GLOBAL_H

#include <stdbool.h>
#include <ir/rir_value.h>
#include <ir/rir_type.h>

// TODO: Rir global could be renamed to something else ...?

struct rirtostr_ctx;

//! A global variable declaration for a module
struct rir_global {
    //! The value of the variable
    struct rir_value val;
};

struct rir_object *rir_global_create(struct rir_type *type,
                                     const struct RFstring *name,
                                     const void *value,
                                     struct rir_ctx *ctx);
void rir_global_deinit(struct rir_global *global);
bool rir_global_tostring(struct rirtostr_ctx *ctx, const struct rir_global *g);

i_INLINE_DECL const struct RFstring *rir_global_name(const struct rir_global *g)
{
    return &g->val.id;
}

i_INLINE_DECL struct rir_type *rir_global_type(const struct rir_global *g)
{
    return g->val.type;
}

/**
 * Add or retrieve a global string literal object
 *
 * @param ctx            The rir context with which to work
 * @param s              The string for which to check if there is a global string
 *                       object in the map and if not to create a map entry. A copy
 *                       of the string is kept inside the global string literal
 *                       object
 * @return               Either the newly created object or the retrieved object.
 *                       NULL in failure
 */
struct rir_object *rir_global_addorget_string(struct rir_ctx *ctx, const struct RFstring *s);

#endif
