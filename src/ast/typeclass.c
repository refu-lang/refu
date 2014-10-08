#include <ast/typeclass.h>

struct ast_node *ast_typeclass_create(struct inplocation_mark *start,
                                      struct inplocation_mark *end,
                                      struct ast_node *name,
                                      struct ast_node *genr)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_TYPECLASS_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, typeclass.name);
    ast_node_register_child(ret, genr, typeclass.generics);

    return ret;
}

struct ast_node *ast_typeinstance_create(struct inplocation_mark *start,
                                         struct inplocation_mark *end,
                                         struct ast_node *class_name,
                                         struct ast_node *type_name,
                                         struct ast_node *genr)
{
    struct ast_node *ret;
    RF_ASSERT(class_name->type == AST_IDENTIFIER);
    RF_ASSERT(type_name->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_TYPECLASS_INSTANCE, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, class_name, typeinstance.class_name);
    ast_node_register_child(ret, type_name, typeinstance.type_name);
    ast_node_register_child(ret, genr, typeinstance.generics);

    return ret;
}

