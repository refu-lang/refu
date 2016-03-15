#ifndef LFR_AST_TYPE_H
#define LFR_AST_TYPE_H

#include <rflib/datastructs/intrusive_list.h>
#include <rflib/utils/container_of.h>

#include <ast/ast.h>

struct analyzer;

/* -- functions concerning both type description and operators */

i_INLINE_DECL struct ast_node *ast_types_left(const struct ast_node *n)
{
    switch (n->type) {
    case AST_TYPE_LEAF:
        return n->typeleaf.left;
    case AST_TYPE_OPERATOR:
        return n->typeop.left;
    default:
        RF_ASSERT_OR_CRITICAL(false, return NULL, "Attempted to call accessor"
                              "for illegal ast node type \""RFS_PF"\"",
                              RFS_PA(ast_node_str(n)));
    }
}

i_INLINE_DECL struct ast_node *ast_types_right(const struct ast_node *n)
{
    switch (n->type) {
    case AST_TYPE_DESCRIPTION:
        return n->typeleaf.right;
    case AST_TYPE_LEAF:
        return n->typeop.right;
    default:
        RF_ASSERT_OR_CRITICAL(false, return NULL, "Attempted to call accessor"
                              "for illegal ast node type \""RFS_PF"\"",
                              RFS_PA(ast_node_str(n)));
    }
}

/**
 * Traverses an ast node type along with an actual type. If they don't have an
 * equal layout false will be returned. If they are do and a  callback has been
 * given, it will be ran for each argument.
 *
 * @param n        The ast node containing the type description
 * @param t        The type to use for the traversal
 * @param cb       An optional callback to call for each final argument of the type.
 *                 The argument's type name is taken from the corresponding ast node
 *                 and the type from traversing the given type. If a callback
 *                 returns false, then the iteration stops.
 * @param user     An optional user argument to provide to the callback
 * @return         true if they are equal and if all callback calls returned true.
 */
typedef bool(*ast_type_cb)(const struct RFstring *name, const struct ast_node *desc, struct type *t, void *user);
bool ast_type_foreach_arg(const struct ast_node *n, struct type *t, ast_type_cb cb, void *user);

/* -- type leaf functions -- */
struct ast_node *ast_typeleaf_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right);

i_INLINE_DECL struct ast_node *ast_typeleaf_left(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_LEAF);
    return n->typeleaf.left;
}

i_INLINE_DECL struct ast_node *ast_typeleaf_right(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_LEAF);
    return n->typeleaf.right;
}

i_INLINE_DECL const struct RFstring *ast_typeleaf_str(const struct ast_node *n)
{
    return ast_identifier_str(ast_typeleaf_left(n));
}

/* -- type operator functions -- */

struct ast_node *ast_typeop_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right);
/**
 * If typeop was not initialized with a right node this function
 * will set both the right child ast node and set typeop's end location
 */
void ast_typeop_set_right(struct ast_node *n, struct ast_node *r);

i_INLINE_DECL enum typeop_type ast_typeop_op(const struct ast_node *n)
{
    return n->typeop.type;
}

/**
 * Return the string representing the operation's type
 */
const struct RFstring *ast_typeop_opstr(const struct ast_node *n);
const struct RFstring *type_op_str(enum typeop_type op);

i_INLINE_DECL struct ast_node *ast_typeop_left(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);
    return n->typeop.left;
}

i_INLINE_DECL struct ast_node *ast_typeop_right(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);
    return n->typeop.right;
}

/* -- type description functions -- */

struct ast_node *ast_typedesc_create(struct ast_node *desc);

i_INLINE_DECL struct symbol_table *ast_typedesc_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    return &n->typedesc.st;
}

i_INLINE_DECL struct ast_node *ast_typedesc_desc_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    return n->typedesc.desc;
}

/**
 * Check that the type description is regarding a simple identifier.
 */
i_INLINE_DECL bool ast_typedesc_type_is_single_identifier(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_TYPE_DESCRIPTION || n->type == AST_TYPE_OPERATOR,
    "Unexpected ast node type");
    struct ast_node *left = ast_types_left(n);

    if (n->type == AST_TYPE_DESCRIPTION && left &&
        !ast_types_right(n) && left->type == AST_IDENTIFIER) {
        return true;
    }

    return false;
}

/**
 * Check that the type description is regarding a composite type
 */
i_INLINE_DECL bool ast_typedesc_type_is_composite(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_TYPE_DESCRIPTION || n->type == AST_TYPE_OPERATOR,
          "Unexpected ast node type");
    struct ast_node *left = ast_types_left(n);

    if (n->type == AST_TYPE_DESCRIPTION && left &&
        !ast_types_right(n) && left->type == AST_IDENTIFIER) {
        return true;
    }

    return false;
}

/* -- type declaration functions -- */

struct ast_node *ast_typedecl_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc);

i_INLINE_DECL const struct RFstring *ast_typedecl_name_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return ast_identifier_str(n->typedecl.name);
}

i_INLINE_DECL struct ast_node *ast_typedecl_typedesc_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return n->typedecl.desc;
}

i_INLINE_DECL struct ast_node *ast_typedecl_genrdecl_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return n->typedecl.genrdecl;
}
#endif
