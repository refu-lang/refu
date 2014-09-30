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
    RF_ASSERT(op->type == AST_BINARY_OPERATOR);
    ast_node_add_child(op, r);
    op->binaryop.right = r;
    ast_node_set_end(op, ast_node_endmark(r));
}

enum binaryop_type binaryop_type_from_token(struct token *tok);

struct ast_node *ast_unaryop_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    enum unaryop_type type,
                                    struct ast_node *operand);

enum unaryop_type unaryop_type_from_token(struct token *tok);
#endif
