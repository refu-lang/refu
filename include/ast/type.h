#ifndef LFR_AST_TYPE_H
#define LFR_AST_TYPE_H

#include <RFintrusive_list.h>
#include <Utils/container_of.h>

#include <ast/ast.h>

struct analyzer;


/* -- functions concerning both type description and operators */

i_INLINE_DECL struct ast_node *ast_types_left(struct ast_node *n)
{
    switch (n->type) {
    case AST_TYPE_DESCRIPTION:
        return n->typedesc.left;
    case AST_TYPE_OPERATOR:
        return n->typeop.left;
    default:
        RF_ASSERT_OR_CRITICAL(false, "Attempted to call accessor"
                              "for illegal ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(n)));
        return NULL;
    }
}

i_INLINE_DECL struct ast_node *ast_types_right(struct ast_node *n)
{
    switch (n->type) {
    case AST_TYPE_DESCRIPTION:
        return n->typedesc.right;
    case AST_TYPE_OPERATOR:
        return n->typeop.right;
    default:
        RF_ASSERT_OR_CRITICAL(false, "Attempted to call accessor"
                              "for illegal ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(n)));
        return NULL;
    }
}

/* -- type operator functions -- */

struct ast_node *ast_typeop_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right);
/**
 * If typeop was not initialized with a right node this function
 * will set both the right child ast node and set typeop's end location
 */
void ast_typeop_set_right(struct ast_node *n, struct ast_node *r);

i_INLINE_DECL enum typeop_type ast_typeop_op(struct ast_node *n)
{
    return n->typeop.type;
}

/**
 * Return the string representing the operation's type
 */
const struct RFstring *ast_typeop_opstr(struct ast_node *n);
const struct RFstring *type_op_str(enum typeop_type op);

i_INLINE_DECL struct ast_node *ast_typeop_left(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);
    return n->typeop.left;
}

i_INLINE_DECL struct ast_node *ast_typeop_right(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);
    return n->typeop.right;
}

/* -- type description functions -- */

struct ast_node *ast_typedesc_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right);

void ast_typedesc_set_left(struct ast_node *n, struct ast_node *l);
void ast_typedesc_set_right(struct ast_node *n, struct ast_node *r);

i_INLINE_DECL struct ast_node *ast_typedesc_left(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    return n->typedesc.left;
}

i_INLINE_DECL struct ast_node *ast_typedesc_right(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    return n->typedesc.right;
}

i_INLINE_DECL struct type *ast_typedesc_type_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    return n->typedesc.type;
}

i_INLINE_DECL void ast_typedesc_type_set(struct ast_node *n, struct type *t)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION);
    n->typedesc.type = t;
}


/**
 * Check that the type description is regarding a simple identifier.
 */
i_INLINE_DECL bool ast_typedesc_type_is_single_identifier(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    struct ast_node *left = ast_types_left(n);

    if (n->type == AST_TYPE_DESCRIPTION && left &&
        !ast_types_right(n) && left->type == AST_IDENTIFIER) {
        return true;
    }

    return false;
}

/**
 * Check that the type description is regarding an anonymous (complex) type
 */
i_INLINE_DECL bool ast_typedesc_type_is_anonymous(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    struct ast_node *left = ast_types_left(n);

    if (n->type == AST_TYPE_DESCRIPTION && left &&
        !ast_types_right(n) && left->type == AST_IDENTIFIER) {
        return true;
    }

    return false;
}

/* -- type declaration functions -- */

struct ast_node *ast_typedecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc);

i_INLINE_DECL const struct RFstring *ast_typedecl_name_str(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return ast_identifier_str(n->typedecl.name);
}

i_INLINE_DECL struct ast_node *ast_typedecl_typedesc_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return n->typedecl.desc;
}

i_INLINE_DECL struct ast_node *ast_typedecl_genrdecl_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    return n->typedecl.genrdecl;
}
#endif
