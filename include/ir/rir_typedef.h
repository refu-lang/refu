#ifndef LFR_IR_RIR_TYPEDEF_H
#define LFR_IR_RIR_TYPEDEF_H

#include <stdbool.h>
#include <ir/rir_argument.h>
#include <Data_Structures/darray.h>

struct rir_ctx;
struct rir_fndef;
struct type;

struct rir_typedef {
    struct RFstring name;
    bool is_union;
    struct rir_type_arr argument_types;
    //! Control to be entered into the rir typedefs list
    struct RFilist_node ln;
};

struct rir_typedef *rir_typedef_create_from_type(struct type *t, struct rir_ctx *ctx);

struct rir_object *rir_typedef_create_obj(
    struct rir *r,
    struct rir_fndef *curr_fn,
    const struct RFstring *name,
    bool is_union,
    const struct rir_type_arr *args
);
struct rir_typedef *rir_typedef_create(
    struct rir *r,
    struct rir_fndef *curr_fn,
    const struct RFstring *name,
    bool is_union,
    const struct rir_type_arr *args
);

void rir_typedef_deinit(struct rir_typedef *t);

bool rir_typedef_tostring(struct rirtostr_ctx *ctx, struct rir_typedef *t);
bool rir_typedef_equal(const struct rir_typedef *t1, const struct rir_typedef *t2);
const struct rir_type *rir_typedef_typeat(const struct rir_typedef *t, unsigned int i);

size_t rir_typedef_bytesize(const struct rir_typedef *t);

#endif
