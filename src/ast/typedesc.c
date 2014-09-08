#include <ast/typedesc.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_typeop_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    RF_ASSERT(left->type == AST_TYPE_DESCRIPTION);
    RF_ASSERT(right->type == AST_TYPE_DESCRIPTION);
    ret = ast_node_create_marks(AST_TYPE_OPERATOR, start, end);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->typeop.type = type;
    ast_node_add_child(ret, left);
    ret->typeop.left = left;
    if (right) {
        ast_node_add_child(ret, right);
        ret->typeop.right = right;
    }
    return ret;
}

void ast_typeop_set_right(struct ast_node *op, struct ast_node *r)
{
    RF_ASSERT(op->type == AST_TYPE_OPERATOR);
    ast_node_add_child(op, r);
    op->typeop.right = r;
    ast_node_set_end(op, ast_node_endmark(r));
}

struct ast_node *ast_typedesc_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_TYPE_DESCRIPTION, start, end);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    if (left) {
        ast_node_add_child(ret, left);
        ret->typedesc.left = left;
    }
    if (right) {
        ast_node_add_child(ret, right);
        ret->typedesc.right = right;
    }

    return ret;
}

void ast_typedesc_set_left(struct ast_typedesc *t, struct ast_node *l)
{
    struct ast_node *n;
    n = ast_typedesc_to_node(t);
    ast_node_add_child(n, l);
    t->left = l;
}

void ast_typedesc_set_right(struct ast_typedesc *t, struct ast_node *r)
{
    struct ast_node *n;
    n = ast_typedesc_to_node(t);
    ast_node_add_child(n, r);
    t->right = r;
}
