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
    ast_node_add_child(ret, right);
    ret->typeop.right = right;
    return ret;
}

void ast_typeop_print(struct ast_node *n, int depth, const char *description)
{
    struct ast_node *c;
    int i = 0;
    rf_ilist_for_each(&n->children, c, lh) {
        if (i == 0) {
            ast_print(c, depth + 1, "left");
        } else if (i == 1) {
            ast_print(c, depth + 1, "right");
        } else {
            RF_ASSERT(0);
        }
        i ++;
    }
}


struct ast_node *ast_typedesc_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *id)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_TYPE_DESCRIPTION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }


    RF_ASSERT(id->type == AST_IDENTIFIER);
    ast_node_add_child(ret, id);
    ret->typedesc.left = id;

    return ret;
}

void ast_typedesc_set_right(struct ast_typedesc *t, struct ast_node *r)
{
    struct ast_node *n;
    n = ast_typedesc_to_node(t)
    RF_ASSERT(r->type == AST_TYPE_DESCRIPTION);
    ast_node_add_child(n, r);
    t->right = r;
}

void ast_typedesc_print(struct ast_node *n, int depth, const char *description)
{
    struct ast_node *c;
    int i = 0;
    rf_ilist_for_each(&n->children, c, lh) {
        if (i == 0) {
            ast_print(c, depth + 1, "left");
        } else if (i == 1) {
            ast_print(c, depth + 1, "right");
        } else {
            RF_ASSERT(0);
        }
        i ++;
    }
}
