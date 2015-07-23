#include <ir/rir_function.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/function.h>

static bool rir_fndecl_init(struct rir_fndecl *ret, const struct ast_node *n)
{
    RF_STRUCT_ZERO(ret);
    ret->name = ast_fndecl_name_str(ast_fnimpl_fndecl_get(n));
    darray_init(ret->arguments);
    darray_init(ret->returns);
    return true;
}

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n)
{
    struct rir_fndecl *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndecl_init(ret, n)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_fndecl_deinit(struct rir_fndecl *f)
{
    darray_free(f->arguments);
    darray_free(f->returns);
}

void rir_fndecl_destroy(struct rir_fndecl *f)
{
    rir_fndecl_deinit(f);
    free(f);
}
