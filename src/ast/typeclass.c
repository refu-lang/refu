#include <ast/typeclass.h>

#include <ast/ast.h>

struct ast_node *ast_typeclass_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *name,
                                      struct ast_node *genr)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_TYPECLASS_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, typeclass.name);
    ast_node_register_child(ret, genr, typeclass.generics);

    return ret;
}

struct ast_node *ast_typeinstance_create(const struct inplocation_mark *start,
                                         const struct inplocation_mark *end,
                                         struct ast_node *class_name,
                                         struct ast_node *type_name,
                                         struct ast_node *genr)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(class_name, AST_IDENTIFIER);
    AST_NODE_ASSERT_TYPE(type_name, AST_IDENTIFIER);

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

