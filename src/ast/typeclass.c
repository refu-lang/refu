#include <ast/typeclass.h>

#include <ast/ast.h>

struct ast_node *ast_typeclass_create(
    const struct inplocation_mark *start,
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

i_INLINE_INS struct symbol_table *ast_typeclass_symbol_table_get(struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_typeclass_name_str(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typeclass_generics(const struct ast_node *n);

struct ast_node *ast_typeinstance_create(
    const struct inplocation_mark *start,
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

const struct type *ast_typeinstance_instantiated_type_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_INSTANCE);
    return ast_node_get_type(n->typeinstance.type_name);
}

i_INLINE_INS struct symbol_table *ast_typeinstance_symbol_table_get(struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_typeinstance_name_str(const struct ast_node *n);
