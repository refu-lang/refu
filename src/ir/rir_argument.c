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
    struct rir_argument *a = &obj->arg;
    if (rir_type_is_elementary(type)) {
        rir_ltype_elem_init(&a->type, (enum elementary_type)type->category);
        if (type->name) {
            a->name = type->name;
        } else {
            a->name = &noname;
        }
    } else {
        const struct RFstring *s = type_get_unique_type_str(type->type, true);
        struct rir_typedef *def = rir_typedef_byname(ctx->rir, s);
        RF_ASSERT_OR_EXIT(def, "typedef should have been found by name");
        rir_ltype_comp_init(&a->type, def, false);
        a->name = rf_string_create("GENERATED_NAME");
    }
    return rir_value_variable_init(&a->val, obj, ctx);
}

struct rir_object *rir_argument_create(const struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_ARGUMENT, ctx->rir);
    if (!ret) {
        return NULL;
    }
    rir_argument_init(ret, t, ctx);
    return ret;
}

struct rir_object *rir_argument_create_from_typedef(const struct rir_typedef *d, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_ARGUMENT, ctx->rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_value_variable_init(&ret->arg.val, ret, ctx)) {
        free(ret);
        return NULL;
    }
    ret->arg.name = d->name;
    rir_ltype_comp_init(&ret->arg.type, d, false);
    return ret;
}

void rir_argument_deinit(struct rir_argument *a)
{
    // TODO
}

bool rir_argument_tostring(struct rirtostr_ctx *ctx, const struct rir_argument *arg)
{
    bool ret = true;
    if (arg->type.category == RIR_LTYPE_ELEMENTARY) {
        ret = rf_stringx_append(
            ctx->rir->buff,
            RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                RF_STR_PF_ARG(arg->name),
                RF_STR_PF_ARG(type_elementary_get_str(arg->type.etype))));
    } else {
        rf_stringx_append(ctx->rir->buff, arg->type.tdef->name);
    }

    return ret;
}

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

    for (int i = 0; i < size; ++i) {
        if (!rir_ltype_equal(&darray_item(*arr1, i)->arg.type, &darray_item(*arr2, i)->arg.type)) {
            return false;
        }
    }
    return true;
}

void rir_argsarr_deinit(struct args_arr *arr, struct rir_ctx *ctx)
{
    struct rir_object **arg;
    darray_foreach(arg, *arr) {
        rir_object_listrem_destroy(*arg, ctx);
    }
    darray_free(*arr);
}
