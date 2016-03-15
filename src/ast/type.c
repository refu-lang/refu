#include <ast/type.h>

#include <rflib/utils/sanity.h>
#include <rflib/string/core.h>

#include <ast/ast.h>
#include <ast/identifier.h>

static const struct RFstring op_str_prod_  = RF_STRING_STATIC_INIT(",");
static const struct RFstring op_str_sum_   = RF_STRING_STATIC_INIT("|");
static const struct RFstring op_str_impl_  = RF_STRING_STATIC_INIT("->");

/* -- functions concerning both type description and operators */

i_INLINE_INS struct ast_node *ast_types_left(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_types_right(const struct ast_node *n);

/* -- type leaf functions -- */

struct ast_node *ast_typeleaf_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end,
                                     struct ast_node *left,
                                     struct ast_node *right)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(left, AST_IDENTIFIER);
    ret = ast_node_create_marks(AST_TYPE_LEAF, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }
    ast_node_register_child(ret, left, typeleaf.left);
    ast_node_register_child(ret, right, typeleaf.right);

    return ret;
}

i_INLINE_INS struct ast_node *ast_typeleaf_left(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typeleaf_right(const struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_typeleaf_str(const struct ast_node *n);
/* -- type operator functions -- */

struct ast_node *ast_typeop_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    RF_ASSERT(left->type == AST_TYPE_DESCRIPTION ||
              left->type == AST_TYPE_OPERATOR ||
              left->type == AST_TYPE_LEAF ||
              left->type == AST_XIDENTIFIER,
              "Unexpected ast node type");

    ret = ast_node_create_marks(AST_TYPE_OPERATOR, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->typeop.type = type;
    ast_node_add_child(ret, left);
    ret->typeop.left = left;
    if (right) {
    RF_ASSERT(right->type == AST_TYPE_DESCRIPTION ||
              right->type == AST_TYPE_OPERATOR ||
              right->type == AST_TYPE_LEAF ||
              right->type == AST_XIDENTIFIER,
              "Unexpected ast node type");
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

i_INLINE_INS enum typeop_type ast_typeop_op(const struct ast_node *n);


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
        RF_ASSERT_OR_CRITICAL(false, return NULL,
                              "Unexpected type operator type encountered");
    }
}

const struct RFstring *ast_typeop_opstr(const struct ast_node *n)
{
    return type_op_str(n->typeop.type);
}

i_INLINE_INS struct ast_node *ast_typeop_left(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typeop_right(const struct ast_node *n);

/* -- type description functions -- */

struct ast_node *ast_typedesc_create(struct ast_node *desc)
{
    struct ast_node *ret;

    ret = ast_node_create_loc(AST_TYPE_DESCRIPTION, ast_node_location(desc));
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, desc, typedesc.desc);
    return ret;
}
i_INLINE_INS struct symbol_table *ast_typedesc_symbol_table_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typedesc_desc_get(const struct ast_node *n);

/* -- type declaration functions -- */
struct ast_node *ast_typedecl_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);
    RF_ASSERT(desc->type == AST_TYPE_DESCRIPTION || desc->type == AST_TYPE_OPERATOR,
              "Unexpected ast node type");

    ret = ast_node_create_marks(AST_TYPE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, typedecl.name);
    ast_node_register_child(ret, desc, typedecl.desc);
    return ret;
}

i_INLINE_INS const struct RFstring *ast_typedecl_name_str(const struct ast_node *n);
i_INLINE_INS struct ast_node* ast_typedecl_typedesc_get(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_typedecl_genrdecl_get(const struct ast_node *n);
