#include <ast/generics.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_genrtype_create(struct parser_file *f, char *sp, char *ep,
                                     struct ast_node *type, struct ast_node *id)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_GENERIC_TYPE, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ast_node_add_child(ret, type);
    ret->genrtype.type = type;
    ast_node_add_child(ret, id);
    ret->genrtype.id = id;
    return ret;
}

struct ast_node *ast_genrdecl_create(struct parser_file *f, char *sp, char *ep)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_GENERIC_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }
    return ret;
}

struct ast_node *ast_genrattr_create(struct parser_file *f, char *sp, char *ep)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_GENERIC_ATTRIBUTE, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }
    return ret;
}

