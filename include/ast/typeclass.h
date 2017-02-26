#ifndef LFR_AST_TYPECLASS_H
#define LFR_AST_TYPECLASS_H

struct inplocation_mark;
struct type;

#include <ast/ast.h>
#include <rfbase/utils/sanity.h>

struct ast_node *ast_typeclass_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *name,
    struct ast_node *genr
);

i_INLINE_DECL const struct RFstring *ast_typeclass_name_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_DECLARATION);
    return ast_identifier_str(n->typeclass.name);
}

i_INLINE_DECL struct symbol_table *ast_typeclass_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_DECLARATION);
    return &n->typeclass.st;
}

i_INLINE_DECL struct ast_node *ast_typeclass_name(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_DECLARATION);
    return n->typeclass.name;
}

struct ast_node *ast_typeinstance_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *class_name,
    struct ast_node *type_name,
    struct ast_node *genr
);

i_INLINE_DECL const struct RFstring *ast_typeinstance_name_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_INSTANCE);
    return ast_identifier_str(n->typeinstance.class_name);
}

i_INLINE_DECL struct symbol_table *ast_typeinstance_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_INSTANCE);
    return &n->typeinstance.st;
}

/**
 * Gets the type which instantiates the typeclass
 */
const struct type *ast_typeinstance_instantiated_type_get(struct ast_node* n);

#endif
