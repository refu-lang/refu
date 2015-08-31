#ifndef LFR_IR_RIR_TYPEDEF_H
#define LFR_IR_RIR_TYPEDEF_H

#include <stdbool.h>
#include <ir/rir_argument.h>
#include <Data_Structures/darray.h>

struct rir_ctx;
struct rir_type;

struct rir_typedef {
    struct RFstring *name;
    bool is_union;
    struct args_arr arguments_list;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};

struct rir_typedef *rir_typedef_create(struct rir_type *t, struct rir_ctx *ctx);
void rir_typedef_destroy(struct rir_typedef *t);

bool rir_typedef_tostring(struct rirtostr_ctx *ctx, struct rir_typedef *t);

#endif
