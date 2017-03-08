#include <analyzer/typecheck_typeclass.h>
#include <types/type.h>
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
    // TODO:
    struct type *instantiated_type = ast_typeinstance_instantiated_type_get(n);
    // TODO: add the typeinstance to the instantiated typeclasses for this type

    return TRAVERSAL_CB_OK;
}
