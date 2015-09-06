#include <ir/rir_typedef.h>
#include <ir/rir.h>
#include <ir/rir_type.h>
#include <ir/rir_object.h>
#include <ir/rir_strmap.h>
#include <types/type.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_typedef_init(struct rir_object *obj, struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_typedef *def = &obj->tdef;
    RF_ASSERT(!rir_type_is_elementary(t), "Typedef can't be created from an elementary type");
    RF_ASSERT(t->category != COMPOSITE_IMPLICATION_RIR_TYPE, "Typedef can't be created from an implication type");
    RF_STRUCT_ZERO(def);

    if (t->category == COMPOSITE_RIR_DEFINED) {
        if (rir_type_is_sumtype(t)) {
            def->is_union = true;
        }
        // if it's an actual typedef get the name and proceed to the first and only
        // subtype which is the actual type declaration
        def->name = rf_string_copy_out(t->name);
        RF_ASSERT(darray_size(t->subtypes) == 1, "defined type should have a single subtype");
        t = darray_item(t->subtypes, 0);
        // to avoid double typedef creation for rir_type subtype description mark it
        rir_ctx_visit_type(ctx, t);
    } else {
        RFS_PUSH();
        def->name = rf_string_copy_out(type_get_unique_type_str(t->type, true));
        RFS_POP();
    }
    if (!rir_type_to_arg_array(t, &def->arguments_list, ctx)) {
        return false;
    }
    // finally add the typedef to the rir's strmap
    if (!strmap_add(&ctx->rir->map, def->name, obj)) {
        return false;
    }
    
    return true;
}

static struct rir_object *rir_typedef_create_obj(struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_TYPEDEF, ctx->rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_typedef_init(ret, t, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_typedef *rir_typedef_create(struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_typedef_create_obj(t, ctx);
    return obj ? &obj->tdef : NULL;
}

void rir_typedef_deinit(struct rir_typedef *t)
{
    rf_string_destroy(t->name);
}

bool rir_typedef_tostring(struct rirtostr_ctx *ctx, struct rir_typedef *t)
{
    if (!rf_stringx_append(
            ctx->rir->buff,
            RFS("$"RF_STR_PF_FMT" = %s(",  RF_STR_PF_ARG(t->name), t->is_union ? "uniondef" : "typedef"))) {
        return false;
    }
    if (!rir_argsarr_tostring(ctx, &t->arguments_list)) {
        return false;
    }

    if (!rf_stringx_append_cstr(ctx->rir->buff, ")\n")) {
        return false;
    }
    return true;
}

bool rir_typedef_equal(const struct rir_typedef *t1, const struct rir_typedef *t2)
{
    // Probably we can avoid comparing argument lists since names are guaranteed
    // to be unique
    return rf_string_equal(t1->name, t2->name) && t1->is_union == t2->is_union;
    // if not we would also need to check something like:
    // rir_argsarr_equal(&t1->arguments_list, &t2->arguments_list)
}

const struct rir_argument *rir_typedef_argat(const struct rir_typedef *t, unsigned int i)
{
    return darray_size(t->arguments_list) > i ? &darray_item(t->arguments_list, i)->arg : NULL;
}
