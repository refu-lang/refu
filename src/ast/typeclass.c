#include <ast/typeclass.h>

#include <rfbase/string/core.h>

#include <ast/ast.h>
#include <ast/function.h>

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
    struct ast_node *instance_name,
    struct ast_node *type_name,
    bool is_default)
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
    ast_node_register_child(ret, instance_name, typeinstance.instance_name);
    ast_node_register_child(ret, type_name, typeinstance.type_name);
    ret->typeinstance.is_default = is_default;

    return ret;
}

struct type *ast_typeinstance_instantiated_type_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_INSTANCE);
    return (struct type*)ast_node_get_type(n->typeinstance.type_name);
}

struct ast_node* ast_typeinstance_getfn_byname(const struct ast_node *n, const struct RFstring* name)
{
    struct ast_node **child;
    darray_foreach(child, n->children) {
        if (ast_node_type(*child) == AST_FUNCTION_IMPLEMENTATION) {
            if (rf_string_equal(name, ast_fnimpl_namestr_get(*child))) {
                return *child;
            }
        }
    }
    return NULL;
}

i_INLINE_INS struct symbol_table *ast_typeinstance_symbol_table_get(struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_typeinstance_classname_str(const struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_typeinstance_typename_str(const struct ast_node *n);
