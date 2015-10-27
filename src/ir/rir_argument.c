#include <ir/rir_argument.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <types/type.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_typearr_add_single(struct rir_type_arr *arr,
                                   const struct type *type,
                                   enum typearr_create_reason reason,
                                   struct rir_ctx *ctx)
{
    struct rir_ltype *t;
    if (type_is_elementary(type)) {
        t = rir_ltype_elem_create(
            type_elementary(type),
            // If it's a string make sure it's passed by pointer to function calls
            // TODO: at some point do away with this distinction (?)
            reason == ARGARR_AT_FNDECL && type_is_specific_elementary(type, ELEMENTARY_TYPE_STRING)
        );
    } else {
        t = rir_ltype_create_from_type(type, ctx);
    }
    if (!t) {
        return false;
    }
    darray_append(*arr, t);
    return true;
}

bool rir_typearr_from_type(struct rir_type_arr *arr,
                           const struct type *type,
                           enum typearr_create_reason reason,
                           struct rir_ctx *ctx)
{
    RF_ASSERT(!type_is_implop(type), "Called with illegal type");
    struct type **subtype;
    darray_init(*arr);
    switch(type->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_DEFINED:
        return rir_typearr_add_single(arr, type, reason, ctx);
    case TYPE_CATEGORY_OPERATOR:
        darray_foreach(subtype, type->operator.operands) {
            if (!rir_typearr_add_single(arr, *subtype, reason, ctx)) {
                return false;
            }
        }
        return true;
    default:
        RF_CRITICAL_FAIL("Unexpected type category encountered");
        break;
    }
    return false;
}

bool rir_typearr_tostring(struct rirtostr_ctx *ctx, const struct rir_type_arr *arr)
{
    bool ret = false;
    size_t i = 0;
    size_t types_num = darray_size(*arr);
    struct rir_ltype **type;
    RFS_PUSH();
    darray_foreach(type, *arr) {
        if (!rf_stringx_append(ctx->rir->buff, rir_ltype_string(*type))) {
            goto end;
        }
        if (++i != types_num) {
            if (!rf_stringx_append_cstr(ctx->rir->buff, ", ")) {
                goto end;
            }
        }
    }
    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}

bool rir_typearr_equal(const struct rir_type_arr *arr1, const struct rir_type_arr *arr2)
{
    size_t size = darray_size(*arr1);
    if (size != darray_size(*arr2)) {
        return false;
    }

    for (unsigned i = 0; i < size; ++i) {
        if (!rir_ltype_identical(darray_item(*arr1, i), darray_item(*arr2, i))) {
            return false;
        }
    }
    return true;
}

void rir_typearr_deinit(struct rir_type_arr *arr)
{
    struct rir_ltype **t;
    darray_foreach(t, *arr) {
        rir_ltype_destroy(*t);
    }
    darray_free(*arr);
}
