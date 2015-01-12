#ifndef LFR_AST_OPERATORS_H
#define LFR_AST_OPERATORS_H

#include <Definitions/inline.h>
#include <ast/ast.h>
struct token;

struct ast_node *ast_binaryop_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     enum binaryop_type type,
                                     struct ast_node *left,
                                     struct ast_node *right);

i_INLINE_DECL void ast_binaryop_set_right(struct ast_node *op, struct ast_node *r)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    ast_node_add_child(op, r);
    op->binaryop.right = r;
    ast_node_set_end(op, ast_node_endmark(r));
}

i_INLINE_DECL enum binaryop_type ast_binaryop_op(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.type;
}

const struct RFstring *ast_binaryop_operation_name_str(enum binaryop_type op);
const struct RFstring * ast_binaryop_opstr(struct ast_node *op);

enum binaryop_type binaryop_type_from_token(struct token *tok);

i_INLINE_DECL struct ast_node *ast_binaryop_left(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.left;
}

i_INLINE_DECL struct ast_node *ast_binaryop_right(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.right;
}

/* -- unary operator functions -- */

struct ast_node *ast_unaryop_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    enum unaryop_type type,
                                    struct ast_node *operand);

enum unaryop_type unaryop_type_from_token(struct token *tok);
#endif
