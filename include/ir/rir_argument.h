#ifndef LFR_IR_ARGUMENT_H
#define LFR_IR_ARGUMENT_H

#include <stdbool.h>
#include <types/type_decls.h>
#include <Data_Structures/darray.h>
#include <ir/rir_value.h>
#include <ir/rir_type.h>

struct rirtostr_ctx;
struct type;
struct rir;

struct rir_type_arr {darray(struct rir_type*);};

enum typearr_create_reason {
    ARGARR_AT_TYPEDESC,
    ARGARR_AT_FNDECL,
};

/**
 * Turns a type to a rir type array
 *
 * @param type            The type to turn into a rir type array
 * @param arr             The rir type array to create
 * @param reason          The reason for creation. Used to specify where the array
 *                        is going to be used
 * @param ctx             The rir context
 * @return                true for success
 */
bool rir_typearr_from_type(struct rir_type_arr *arr,
                           const struct type *type,
                           enum typearr_create_reason reason,
                           struct rir_ctx *ctx);
bool rir_typearr_tostring(struct rirtostr_ctx *ctx, const struct rir_type_arr *arr);
bool rir_typearr_equal(const struct rir_type_arr *arr1, const struct rir_type_arr *arr2);

void rir_typearr_deinit(struct rir_type_arr *arr);
#endif
