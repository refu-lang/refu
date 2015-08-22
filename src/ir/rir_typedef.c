#include <ir/rir_typedef.h>
#include <ir/rir.h>
#include <ir/rir_type.h>
#include <Utils/memory.h>
#include <String/rf_str_manipulationx.h>

static bool rir_typedef_init(struct rir_typedef *def, struct rir_type *t)
{
    RF_ASSERT(!rir_type_is_elementary(t), "Typedef can't be created from an elementary type");
    darray_init(def->arguments_list);
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
    free(t);
}

bool rir_typedef_tostring(struct rir *r, struct rir_typedef *t)
{
    if (t->is_union) {
        return rf_stringx_append(r->buff, RFS("uniondef()\n"));
    }
    return rf_stringx_append(r->buff, RFS("typedef()\n"));
}
