#ifndef LFR_AST_TYPEDESC_H
#define LFR_AST_TYPEDESC_H

#include <RFintrusive_list.h>
#include <Utils/container_of.h>

struct ast_node;
struct inplocation_mark;

enum typeop_type {
    TYPEOP_INVALID,
    TYPEOP_SUM,
    TYPEOP_PRODUCT,
    TYPEOP_IMPLICATION
};

struct ast_typeop {
    //! Type Operator type
    enum typeop_type type;
    struct ast_node *left;
    struct ast_node *right;
};
#define ast_typeop_to_node(n_)                    \
    container_of((n_), struct ast_node, typeop)

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

struct ast_typedesc {
    struct ast_node *left;
    struct ast_node *right;
};
#define ast_typedesc_to_node(n_)                  \
    container_of((n_), struct ast_node, typedesc)

struct ast_node *ast_typedesc_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right);

void ast_typedesc_set_left(struct ast_typedesc *n, struct ast_node *l);
void ast_typedesc_set_right(struct ast_typedesc *n, struct ast_node *r);


#endif
