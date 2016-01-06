#include <ir/rir_typedef.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_strmap.h>
#include <types/type.h>
#include <types/type_operators.h>
#include <math/math.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_typedef_init_from_type(struct rir_object *obj, struct type *t, struct rir_ctx *ctx)
{
    struct rir_typedef *def = &obj->tdef;
    RF_ASSERT(!type_is_elementary(t), "Typedef can't be created from an elementary type");
    RF_ASSERT(!type_is_implop(t), "Typedef can't be created from an implication type");
    RF_STRUCT_ZERO(def);

    def->is_union = type_is_sumtype(t);
    if (type_is_defined(t)) {
        // if it's an actual typedef get the name and proceed to the first and only
        // subtype which is the actual type declaration
        if (!rf_string_copy_in(&def->name, type_defined_get_name(t))) {
            RF_ERROR("Failed to set copy a string to a typedef");
            return false;
        }
        t = type_defined_get_type(t);
        // set the typedef rir object in the symbol table
        if (!rir_ctx_st_setobj(ctx, &def->name, obj)) {
            RF_ERROR("Failed to set the typedef rir object in the symbol table");
            return false;
        }
    } else {
        RFS_PUSH();
        if (!rf_string_copy_in(&def->name, type_get_unique_type_str(t))) {
            RF_ERROR("Failed to set copy a string to a typedef");
            RFS_POP();
            return false;
        }
        RFS_POP();
        // since this is a new, "internally created" type create a new symbol table record
        if (!rir_ctx_st_newobj(ctx, &def->name, t, obj)) {
            RF_ERROR("Failed to create new symbol table record for a rir created type");
            return false;
        }
    }
    if (!rir_typearr_from_type(&def->argument_types, t, ARGARR_AT_TYPEDESC, ctx)) {
        RF_ERROR("Failed to turn a type to an arg array");
        return false;
    }

    // finally add the typedef to the rir's strmap
    if (!strmap_add(&ctx->common.rir->map, &def->name, obj)) {
        RF_ERROR("Failed to add a typedef to the rir strmap");
        return false;
    }

    return true;
}

static struct rir_object *rir_typedef_create_obj_from_type(struct type *t, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_TYPEDEF, ctx->common.rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_typedef_init_from_type(ret, t, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

struct rir_typedef *rir_typedef_create_from_type(struct type *t, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_typedef_create_obj_from_type(t, ctx);
    return obj ? &obj->tdef : NULL;
}

struct rir_object *rir_typedef_create_obj(
    struct rir *r,
    struct rir_fndef *curr_fn,
    const struct RFstring *name,
    bool is_union,
    const struct rir_type_arr *args
)
{
    struct rir_object *obj = rir_object_create(RIR_OBJ_TYPEDEF, r);
    if (!obj) {
        return NULL;
    }
    struct rir_typedef *def = &obj->tdef;

    if (!rf_string_copy_in(&def->name, name)) {
        goto fail;
    }
    def->is_union = is_union;
    darray_shallow_copy(def->argument_types, *args);

    rf_ilist_add_tail(&r->typedefs, &def->ln);
    return obj;

fail:
    rir_object_listrem_destroy(obj, r, curr_fn);
    return NULL;
}

struct rir_typedef *rir_typedef_create(
    struct rir *r,
    struct rir_fndef *curr_fn,
    const struct RFstring *name,
    bool is_union,
    const struct rir_type_arr *args
)
{
    struct rir_object *obj = rir_typedef_create_obj(r, curr_fn, name, is_union, args);
    return obj ? &obj->tdef : NULL;
}

void rir_typedef_deinit(struct rir_typedef *t)
{
    rf_string_deinit(&t->name);
    rir_typearr_deinit(&t->argument_types, NULL);
}

bool rir_typedef_tostring(struct rirtostr_ctx *ctx, struct rir_typedef *t)
{
    if (!rf_stringx_append(
            ctx->rir->buff,
            RFS("$"RF_STR_PF_FMT" = %s(",  RF_STR_PF_ARG(&t->name), t->is_union ? "uniondef" : "typedef"))) {
        return false;
    }
    if (!rir_typearr_tostring(ctx, &t->argument_types)) {
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
    return rf_string_equal(&t1->name, &t2->name) && t1->is_union == t2->is_union;
    // if not we would also need to check something like:
    // rir_argsarr_equal(&t1->arguments_list, &t2->arguments_list)
}

const struct rir_type *rir_typedef_typeat(const struct rir_typedef *t, unsigned int i)
{
    return darray_size(t->argument_types) > i ? darray_item(t->argument_types, i) : NULL;
}

size_t rir_typedef_bytesize(const struct rir_typedef *t)
{
    struct rir_type **argtype;
    size_t sz;
    if (t->is_union) {
        //find size of biggest type of the union
        sz = 0;
        darray_foreach(argtype, t->argument_types) {
            sz = rf_max(sz, rir_type_bytesize(*argtype));
        }
        return sz + sizeof(uint32_t); // size is that, plus size of the index
    }
    // else size of a normal typedef is sum of all its type sizes
    sz = 0;
    darray_foreach(argtype, t->argument_types) {
        sz += rir_type_bytesize(*argtype);
    }
    return sz;
}
