#ifndef LFR_IR_ARGUMENT_H
#define LFR_IR_ARGUMENT_H

#include <stdbool.h>

#include <rflib/datastructs/darray.h>

#include <types/type_decls.h>
#include <ir/rir_value.h>
#include <ir/rir_type.h>
#include <ir/rir_utils.h>

struct rirtostr_ctx;
struct type;
struct rir;

struct rir_type_arr {darray(struct rir_type*);};

/**
 * Turns a type to a rir type array
 *
 * @param type            The type to turn into a rir type array
 * @param arr             The rir type array to create
 * @param loc             The code location of the type creation. Used to
 *                        specify where the array is going to be used
 * @param ctx             The rir context
 * @return                true for success
 */
bool rir_typearr_from_type(
    struct rir_type_arr *arr,
    const struct type *type,
    enum rir_code_loc loc,
    struct rir_ctx *ctx
);
bool rir_typearr_tostring(struct rirtostr_ctx *ctx, const struct rir_type_arr *arr);
bool rir_typearr_equal(const struct rir_type_arr *arr1, const struct rir_type_arr *arr2);

/**
 * Frees a type array
 *
 * @param arr            The rir type array to free
 * @param r              The rir context or NULL. If the rir context is given
 *                       then also the types of the array are individually freed
 *                       since then the function has access to the memory pool
 */
void rir_typearr_deinit(struct rir_type_arr *arr, struct rir *r);
#endif
