#include <ir/rir_function.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ir/rir_block.h>

static bool rir_fndecl_init(struct rir_fndecl *ret,
                            const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(ret);
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    ret->name = ast_fndecl_name_str(ast_fnimpl_fndecl_get(n));
    ret->body = rir_block_create(ast_fnimpl_body_get(n), 0, ctx);
    strmap_init(&ret->map);
    darray_init(ret->arguments);
    darray_init(ret->returns);
    return true;
}

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_fndecl *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndecl_init(ret, n, ctx)) {
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
