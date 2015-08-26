#include <ir/rir_typedef.h>
#include <ir/rir.h>
#include <ir/rir_type.h>
#include <types/type.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>
#include <String/rf_str_core.h>

static bool rir_typedef_init(struct rir_typedef *def, struct rir_type *t)
{
    RF_ASSERT(!rir_type_is_elementary(t), "Typedef can't be created from an elementary type");
    RF_ASSERT(t->category != COMPOSITE_IMPLICATION_RIR_TYPE, "Typedef can't be created from an implication type");
    RF_STRUCT_ZERO(def);
    RFS_PUSH();
    def->name = rf_string_copy_out(type_get_unique_type_str(t->type, true));
    RFS_POP();
    if (rir_type_is_sumtype(t)) {
        def->is_union = true;
    }

    if (!rir_type_to_arg_array(t, &def->arguments_list)) {
        return false;
    }
    
    return true;
}

struct rir_typedef *rir_typedef_create(struct rir_type *t)
{
    struct rir_typedef *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_typedef_init(ret, t)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_typedef_destroy(struct rir_typedef *t)
{
    rf_string_destroy(t->name);
    free(t);
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
