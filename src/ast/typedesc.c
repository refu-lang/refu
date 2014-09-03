#include <ast/typedesc.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

i_INLINE_INS const struct RFstring *typeop_type_str(enum typeop_type type);

struct ast_node *ast_typeop_create(struct parser_file *f,
                                   char *sp,
                                   char *ep,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    RF_ASSERT(left->type == AST_TYPE_DESCRIPTION);
    RF_ASSERT(right->type == AST_TYPE_DESCRIPTION);
    ret = ast_node_create(AST_TYPE_OPERATOR, f, sp, ep);
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
    ast_node_set_end(op, ast_node_endsp(r));
}

struct ast_node *ast_typedesc_create(struct parser_file *f,
                                      char *sp,
                                      char *ep,
                                      struct ast_node *left,
                                      struct ast_node *right)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_TYPE_DESCRIPTION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ast_node_add_child(ret, left);
    ret->typedesc.left = left;
    if (right) {
        ast_node_add_child(ret, right);
        ret->typedesc.right = right;
    }

    return ret;
}

void ast_typedesc_set_right(struct ast_typedesc *t, struct ast_node *r)
{
    struct ast_node *n;
    n = ast_typedesc_to_node(t);
    ast_node_add_child(n, r);
    t->right = r;
}
