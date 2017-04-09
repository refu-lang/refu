#include <analyzer/typecheck_typeclass.h>
#include <analyzer/analyzer.h>
#include <types/type.h>
#include <module.h>
#include <ast/typeclass.h>

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
    // TODO: add the typeinstance to the instantiated typeclasses for this type
    const struct type *instantiated_type = ast_typeinstance_instantiated_type_get(n);
    module_add_type_instance(ctx->m, n);
    return TRAVERSAL_CB_OK;
}
