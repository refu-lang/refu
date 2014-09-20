#ifndef LFR_AST_TYPE_H
#define LFR_AST_TYPE_H

#include <RFintrusive_list.h>
#include <Utils/container_of.h>

#include <ast/ast.h>

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



/* -- type description functions -- */

struct ast_node *ast_typedesc_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right);

void ast_typedesc_set_left(struct ast_node *n, struct ast_node *l);
void ast_typedesc_set_right(struct ast_node *n, struct ast_node *r);


/* -- type declaration functions -- */

struct ast_node *ast_typedecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc);

i_INLINE_DECL struct RFstring *ast_typedecl_name_str(struct ast_node *n)
{
    return ast_identifier_str(n->typedecl.name);
}
#endif
