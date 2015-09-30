#ifndef LFR_IR_ARGUMENT_H
#define LFR_IR_ARGUMENT_H

#include <stdbool.h>
#include <types/type_decls.h>
#include <Data_Structures/darray.h>
#include <ir/rir_value.h>
#include <ir/rir_ltype.h>

struct rirtostr_ctx;
struct rir_type;
struct rir;

//! Represents a leaf argument on the IR. e.g. a:i32
struct rir_argument {
    //! An optional name for the argument. Always point to other strings. No need for destruction.
    const struct RFstring *name;
    //! The argument's value
    struct rir_value val;
};

struct rir_object *rir_argument_create(const struct rir_type *type, struct rir_ctx *ctx);
struct rir_object *rir_argument_create_from_typedef(const struct rir_typedef *d, struct rir_ctx *ctx);
void rir_argument_deinit(struct rir_argument *a);
bool rir_argument_tostring(struct rirtostr_ctx *ctx, const struct rir_argument *arg);

i_INLINE_DECL struct rir_ltype *rir_argument_type(const struct rir_argument *arg)
{
    return arg->val.type;
}

/* -- Functions dealing with argument arrays -- */

struct args_arr {darray(struct rir_object*);};
bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr, struct rir_ctx *ctx);
bool rir_argsarr_tostring(struct rirtostr_ctx *ctx, const struct args_arr *arr);
bool rir_argsarr_equal(const struct args_arr *arr1, const struct args_arr *arr2);
/**
 * Free a rir_arguments array, freeing each argument's memory individually
 * and making sure they are remove from the global rir_objects array
 */
void rir_argsarr_deinit_remobjs(struct args_arr *arr, struct rir_ctx *ctx);
/**
 * Free a rir_arguments array, without touching the individual arguments
 */
void rir_argsarr_deinit(struct args_arr *arr);
#endif
