#include <ir/rir_typedef.h>
#include <ir/rir.h>
#include <ir/rir_type.h>
#include <ir/rir_object.h>
#include <ir/rir_strmap.h>
#include <types/type.h>
#include <math/math.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_typedef_init(struct rir_object *obj, struct rir_type *t, struct rir_ctx *ctx)
{
    struct rir_typedef *def = &obj->tdef;
    RF_ASSERT(!rir_type_is_elementary(t), "Typedef can't be created from an elementary type");
    RF_ASSERT(t->category != COMPOSITE_IMPLICATION_RIR_TYPE, "Typedef can't be created from an implication type");
    RF_STRUCT_ZERO(def);

    def->is_union = rir_type_is_sumtype(t);
    if (t->category == COMPOSITE_RIR_DEFINED) {
        // if it's an actual typedef get the name and proceed to the first and only
        // subtype which is the actual type declaration
        def->name = rf_string_copy_out(t->name);
        RF_ASSERT(darray_size(t->subtypes) == 1, "defined type should have a single subtype");
        t = darray_item(t->subtypes, 0);
        // set the typedef rir object in the symbol table
        if (!rir_ctx_st_setobj(ctx, def->name, obj)) {
            RF_ERROR("Failed to set the typedef rir object in the symbol table");
            return false;
        }
    } else {
        RFS_PUSH();
        def->name = rf_string_copy_out(type_get_unique_type_str(t->type, true));
        RFS_POP();
        // since this is a new, "internally created" type create a new symbol table record
        if (!rir_ctx_st_newobj(ctx, def->name, (struct type*)t->type, obj)) {
            RF_ERROR("Failed to create new symbol table record for a rir created type");
            return false;
        }
    }
    if (!rir_type_to_arg_array(t, &def->arguments_list, ctx)) {
        RF_ERROR("Failed to turn a type to an arg array");
        return false;
    }

    // finally add the typedef to the rir's strmap
    if (!strmap_add(&ctx->rir->map, def->name, obj)) {
        RF_ERROR("Failed to add a typedef to the rir strmap");
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
    rir_argsarr_deinit(&t->arguments_list);
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

size_t rir_typedef_bytesize(const struct rir_typedef *t)
{
    struct rir_object **arg;
    struct rir_ltype *argtype;
    size_t sz;
    if (t->is_union) {
        //find size of biggest type of the union
        sz = 0;
        darray_foreach(arg, t->arguments_list) {
            argtype = rir_argument_type(&(*arg)->arg);
            sz = rf_max(sz, rir_ltype_bytesize(argtype));
        }
        return sz + sizeof(uint32_t); // size is that, plus size of the index
    }
    // else size of a normal typedef is sum of all its type sizes
    sz = 0;
    darray_foreach(arg, t->arguments_list) {
        argtype = rir_argument_type(&(*arg)->arg);
        sz += rir_ltype_bytesize(argtype);
    }
    return sz;
}
