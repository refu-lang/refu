#ifndef LFR_IR_RIR_VARIABLE_H
#define LFR_IR_RIR_VARIABLE_H

#include <stdbool.h>
#include <ir/rir_value.h>


struct rirtostr_ctx;
struct rir_ltype;

struct rir_variable {
    //! The value of the variable
    struct rir_value val;
};

struct rir_object *rir_variable_create(struct rir_ltype *type, struct rir_ctx *ctx);
void rir_variable_deinit(struct rir_variable *var);
bool rir_variale_tostring(struct rirtostr_ctx *ctx, const struct rir_variable *g);

#endif
