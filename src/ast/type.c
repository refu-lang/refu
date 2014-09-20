#include <ast/type.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

static const struct RFstring op_str_prod_  = RF_STRING_STATIC_INIT(",");
static const struct RFstring op_str_sum_   = RF_STRING_STATIC_INIT("|");
static const struct RFstring op_str_impl_  = RF_STRING_STATIC_INIT("->");

/* -- type operator functions -- */

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

i_INLINE_INS enum typeop_type ast_typeop_op(struct ast_node *n);
const struct RFstring *ast_typeop_opstr(struct ast_node *n)
{
    switch(n->typeop.type) {
    case TYPEOP_PRODUCT:
        return &op_str_prod_;
    case TYPEOP_SUM:
        return &op_str_sum_;
    case TYPEOP_IMPLICATION:
        return &op_str_impl_;
    default:
        RF_ASSERT(0);
        return NULL;
    }
}

/* -- type description functions -- */

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

void ast_typedesc_set_left(struct ast_node *n, struct ast_node *l)
{
    ast_node_add_child(n, l);
    n->typedesc.left = l;
}

void ast_typedesc_set_right(struct ast_node *n, struct ast_node *r)
{
    ast_node_add_child(n, r);
    n->typedesc.right = r;
}

/* -- type declaration functions -- */
struct ast_node *ast_typedecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(desc->type == AST_TYPE_DESCRIPTION);

    ret = ast_node_create_marks(AST_TYPE_DECLARATION, start, end);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ast_node_add_child(ret, name);
    ret->typedecl.name = name;
    ast_node_add_child(ret, desc);
    ret->typedecl.desc = desc;
    return ret;
}
i_INLINE_INS struct RFstring *ast_typedecl_name_str(struct ast_node *n);
