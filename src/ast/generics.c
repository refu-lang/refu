#include <ast/generics.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_genrtype_create(struct ast_node *type, struct ast_node *id)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_TYPE, ast_node_startmark(type),
                                ast_node_endmark(id));
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_add_child(ret, type);
    ret->genrtype.type = type;
    ast_node_add_child(ret, id);
    ret->genrtype.id = id;
    return ret;
}

struct ast_node *ast_genrdecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }
    return ret;
}

struct ast_node *ast_genrattr_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_ATTRIBUTE, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }
    return ret;
}

