#include <ir/rir_argument.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <types/type.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_argument_init(struct rir_object *obj, const struct rir_type *type, struct rir_ctx *ctx)
{
    static const struct RFstring noname = RF_STRING_STATIC_INIT("noname");
    static const struct RFstring genname = RF_STRING_STATIC_INIT("GENERATED_NAME");
    struct rir_argument *a = &obj->arg;
    struct rir_ltype *argtype;
    if (rir_type_is_elementary(type)) {
        argtype = rir_ltype_elem_create((enum elementary_type)type->category, false);
        if (type->name) {
            a->name = type->name;
        } else {
            a->name = &noname;
        }
    } else {
        const struct RFstring *s = type_get_unique_type_str(type->type, true);
        struct rir_typedef *def = rir_typedef_byname(ctx->rir, s);
        RF_ASSERT_OR_EXIT(def, "typedef should have been found by name");
        argtype = rir_ltype_comp_create(def, false);
        a->name = &genname;
    }
    return rir_value_variable_init(&a->val, obj, argtype, ctx);
}

struct rir_object *rir_argument_create(const struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_ARGUMENT, ctx->rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_argument_init(ret, t, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_object *rir_argument_create_from_typedef(const struct rir_typedef *d, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_ARGUMENT, ctx->rir);
    if (!ret) {
        return NULL;
    }
    struct rir_ltype *argtype = rir_ltype_comp_create(d, false);
    if (!rir_value_variable_init(&ret->arg.val, ret, argtype, ctx)) {
        free(ret);
        return NULL;
    }
    ret->arg.name = d->name;
    return ret;
}

void rir_argument_deinit(struct rir_argument *a)
{
    rir_value_deinit(&a->val);
}

bool rir_argument_tostring(struct rirtostr_ctx *ctx, const struct rir_argument *arg)
{
    bool ret = true;
    struct rir_ltype *type = rir_argument_type(arg);
    if (type->category == RIR_LTYPE_ELEMENTARY) {
        ret = rf_stringx_append(
            ctx->rir->buff,
            RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                RF_STR_PF_ARG(arg->name),
                RF_STR_PF_ARG(type_elementary_get_str(type->etype))));
    } else {
        rf_stringx_append(ctx->rir->buff, type->tdef->name);
    }
    return ret;
}


i_INLINE_INS struct rir_ltype *rir_argument_type(const struct rir_argument *arg);




/* -- Functions dealing with argument arrays -- */

bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr, struct rir_ctx *ctx)
{
    RF_ASSERT(type->category != COMPOSITE_IMPLICATION_RIR_TYPE,
              "Called with illegal rir type");
    struct rir_type **subtype;
    struct rir_object *arg;
    darray_init(*arr);
    if (darray_size(type->subtypes) == 0) {
        if (!rir_type_is_trivial(type)) {
            if (!(arg = rir_argument_create(type, ctx))) {
                return false;
            }
            darray_append(*arr, arg);
        }
    } else if (type->category != COMPOSITE_IMPLICATION_RIR_TYPE) {
        darray_foreach(subtype, type->subtypes) {
            if (!rir_type_is_trivial(*subtype)) {
                if (!(arg = rir_argument_create(*subtype, ctx))) {
                    return false;
                }
                darray_append(*arr, arg);
            }
        }
    }
    return true;
}

bool rir_argsarr_tostring(struct rirtostr_ctx *ctx, const struct args_arr *arr)
{
    size_t i = 0;
    size_t args_num = darray_size(*arr);
    struct rir_object **arg;
    darray_foreach(arg, *arr) {
        RF_ASSERT((*arg)->category == RIR_OBJ_ARGUMENT, "Expected a rir argument object");
        if (!rir_argument_tostring(ctx, &(*arg)->arg)) {
            return false;
        }
        if (++i != args_num) {
            if (!rf_stringx_append_cstr(ctx->rir->buff, ", ")) {
                return false;
            }
        }
    }
    return true;
}

bool rir_argsarr_equal(const struct args_arr *arr1, const struct args_arr *arr2)
{
    size_t size = darray_size(*arr1);
    if (size != darray_size(*arr2)) {
        return false;
    }

    for (unsigned i = 0; i < size; ++i) {
        if (!rir_ltype_equal(rir_argument_type(&darray_item(*arr1, i)->arg),
                             rir_argument_type(&darray_item(*arr2, i)->arg))) {
            return false;
        }
    }
    return true;
}

void rir_argsarr_deinit_remobjs(struct args_arr *arr, struct rir_ctx *ctx)
{
    struct rir_object **arg;
    darray_foreach(arg, *arr) {
        rir_object_listrem_destroy(*arg, ctx);
    }
    darray_free(*arr);
}

void rir_argsarr_deinit(struct args_arr *arr)
{
    darray_free(*arr);
}
