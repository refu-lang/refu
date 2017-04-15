#include <analyzer/typecheck_typeclass.h>
#include <analyzer/analyzer.h>
#include <types/type.h>
#include <module.h>
#include <ast/typeclass.h>

bool typeclass_ctx_init(struct typeclass_ctx *ctx, struct type *t)
{
    ctx->instantiated_type = t;
    return true;
}

enum traversal_cb_res typecheck_typeclass(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // TODO:
    return TRAVERSAL_CB_OK;
}

enum traversal_cb_res typecheck_typeinstance(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *instantiated_type = ast_typeinstance_instantiated_type_get(n);
    module_add_type_instance(ctx->m, n);
    return TRAVERSAL_CB_OK;
}
