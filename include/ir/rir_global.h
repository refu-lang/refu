#ifndef LFR_IR_RIR_GLOBAL_H
#define LFR_IR_RIR_GLOBAL_H

#include <stdbool.h>
#include <ir/rir_value.h>
#include <ir/rir_ltype.h>

struct rirtostr_ctx;

//! A global variable declaration for a module
struct rir_global {
    
    //! The value of the variable
    struct rir_value val;
};


struct rir_object *rir_global_create(struct rir_ltype *type,
                                     const struct RFstring *name,
                                     void *value,
                                     struct rir_ctx *ctx);
void rir_global_deinit(struct rir_global *global);
bool rir_global_tostring(struct rirtostr_ctx *ctx, const struct rir_global *g);

i_INLINE_DECL const struct RFstring *rir_global_name(const struct rir_global *g)
{
    return &g->val.id;
}

i_INLINE_DECL struct rir_ltype *rir_global_type(const struct rir_global *g)
{
    return g->val.type;
}

#endif
