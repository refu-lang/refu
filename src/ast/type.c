#include <ast/type.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

static const struct RFstring op_str_prod_  = RF_STRING_STATIC_INIT(",");
static const struct RFstring op_str_sum_   = RF_STRING_STATIC_INIT("|");
static const struct RFstring op_str_impl_  = RF_STRING_STATIC_INIT("->");

/* -- functions concerning both type description and operators */

i_INLINE_INS struct ast_node *ast_types_left(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_types_right(struct ast_node *n);

/* -- type operator functions -- */

struct ast_node *ast_typeop_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(left, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);

    ret = ast_node_create_marks(AST_TYPE_OPERATOR, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->typeop.type = type;
    ast_node_add_child(ret, left);
    ret->typeop.left = left;
    if (right) {
        AST_NODE_ASSERT_TYPE(right, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
        ast_node_add_child(ret, right);
        ret->typeop.right = right;
    }

    return ret;
}

void ast_typeop_set_right(struct ast_node *op, struct ast_node *r)
{
    AST_NODE_ASSERT_TYPE(op, AST_TYPE_OPERATOR);
    ast_node_add_child(op, r);
    op->typeop.right = r;
    ast_node_set_end(op, ast_node_endmark(r));
}

i_INLINE_INS enum typeop_type ast_typeop_op(struct ast_node *n);


const struct RFstring *type_op_str(enum typeop_type op)
{
    switch(op) {
    case TYPEOP_PRODUCT:
        return &op_str_prod_;
    case TYPEOP_SUM:
        return &op_str_sum_;
    case TYPEOP_IMPLICATION:
        return &op_str_impl_;
    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Unexpected type operator type encountered");
        return NULL;
    }
}

const struct RFstring *ast_typeop_opstr(struct ast_node *n)
{
    return type_op_str(n->typeop.type);
}

i_INLINE_INS struct ast_node *ast_typeop_left(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typeop_right(struct ast_node *n);

/* -- type description functions -- */

struct ast_node *ast_typedesc_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_TYPE_DESCRIPTION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    if (left) {
        ast_node_register_child(ret, left, typedesc.left);
    }
    if (right) {
        ast_node_register_child(ret, right, typedesc.right);
    }

    ret->typedesc.type = NULL;

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

i_INLINE_INS struct ast_node *ast_typedesc_left(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typedesc_right(struct ast_node *n);
i_INLINE_INS struct type *ast_typedesc_type_get(struct ast_node *n);
i_INLINE_INS void ast_typedesc_type_set(struct ast_node *n, struct type *t);

/* -- type declaration functions -- */
struct ast_node *ast_typedecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);
    AST_NODE_ASSERT_TYPE(desc, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);

    ret = ast_node_create_marks(AST_TYPE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, typedecl.name);
    ast_node_register_child(ret, desc, typedecl.desc);
    return ret;
}

i_INLINE_INS const struct RFstring *ast_typedecl_name_str(struct ast_node *n);
i_INLINE_INS struct ast_node* ast_typedecl_typedesc_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typedecl_genrdecl_get(struct ast_node *n);
i_INLINE_INS struct symbol_table *ast_typedecl_symbol_table_get(struct ast_node *n);
