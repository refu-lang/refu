#ifndef LFR_AST_OPERATORS_H
#define LFR_AST_OPERATORS_H

#include <rflib/defs/inline.h>
#include <ast/ast.h>

struct token;

struct ast_node *ast_binaryop_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
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

i_INLINE_DECL enum binaryop_type ast_binaryop_op(const struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.type;
}

i_INLINE_DECL bool ast_node_is_specific_binaryop(const struct ast_node *n,
                                                 enum binaryop_type optype)
{
    return n->type == AST_BINARY_OPERATOR && n->binaryop.type == optype;
}

/**
 * Given a binary operation type return a string description of it
 */
const struct RFstring *ast_binaryop_operation_name_str(enum binaryop_type op);
/**
 * Given a binary operation node return a string representation of the operation
 */
const struct RFstring *ast_binaryop_opstr(const struct ast_node *op);

enum binaryop_type binaryop_type_from_token(struct token *tok);

i_INLINE_DECL struct ast_node *ast_binaryop_left(const struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.left;
}

i_INLINE_DECL struct ast_node *ast_binaryop_right(const struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return op->binaryop.right;
}

/**
 * Returns the common type that both operands of a binary operation can have
 *
 * This function should only be invoked after typechecking and particularly
 * from the backend in order to find common type of a binary operation to cast
 * both operands to.
 *
 * @param op        The binary operator node
 * @return          The common type of both operands
 */
i_INLINE_DECL const struct type *ast_binaryop_common_type(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    AST_NODE_ASSERT_STATE(op, AST_NODE_STATE_RIR_END);
    return op->binaryop.common_type;
}

/* -- unary operator functions -- */

struct ast_node *ast_unaryop_create(const struct inplocation_mark *start,
                                    const struct inplocation_mark *end,
                                    enum unaryop_type type,
                                    struct ast_node *operand);

i_INLINE_DECL enum unaryop_type ast_unaryop_op(const struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_UNARY_OPERATOR);
    return op->unaryop.type;
}

i_INLINE_DECL struct ast_node *ast_unaryop_operand(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_UNARY_OPERATOR);
    return op->unaryop.operand;
}

/**
 * Given a unary operation type return a string description of it
 */
const struct RFstring *ast_unaryop_operation_name_str(enum unaryop_type op);
/**
 * Given a unary operation type return a string representation of its type
 */
const struct RFstring *ast_unaryop_opstr(struct ast_node *op);

/**
 * Given a token return its @ref unaryop_type
 */
enum unaryop_type unaryop_type_from_token(struct token *tok);
#endif
