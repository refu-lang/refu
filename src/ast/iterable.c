#include <ast/iterable.h>
#include <ast/ast.h>

struct ast_node *ast_iterable_create_identifier(struct ast_node *identifier)
{
    struct ast_node *ret = ast_node_create_marks(
        AST_ITERABLE,
        ast_node_startmark(identifier),
        ast_node_endmark(identifier)
    );
    if (!ret) {
        return NULL;
    }
    ret->iterable.type = ITERABLE_COLLECTION;
    ast_node_register_child(ret, identifier, iterable.identifier);

    return ret;
}

struct ast_node* ast_iterable_identifier_get(const struct ast_node *it)
{
    RF_ASSERT(
        it->type == AST_ITERABLE && it->iterable.type == ITERABLE_COLLECTION,
        "Illegal ast node type. Expected a collection iterable"
    );
    return it->iterable.identifier;
}
