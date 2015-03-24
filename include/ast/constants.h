#ifndef LFR_AST_CONSTANTS_H
#define LFR_AST_CONSTANTS_H

#include <stdbool.h>
#include <stdint.h>
#include <Definitions/inline.h>

#include <lexer/tokens.h>

struct ast_node;
struct inplocation;

struct ast_node *ast_constant_create_integer(struct inplocation *loc, int64_t value);
struct ast_node *ast_constant_create_float(struct inplocation *loc, double value);
// TODO: Maybe just have 2 of those nodes inside the parser and point at them when needed?
struct ast_node *ast_constant_create_boolean(const struct inplocation *loc, bool value);
i_INLINE_DECL struct ast_node *ast_constant_create_boolean_from_tok(struct token *tok)
{
    RF_ASSERT(tok->type == TOKEN_KW_TRUE || tok->type == TOKEN_KW_FALSE,
              "function called with invalid token type");
    return ast_constant_create_boolean(token_get_loc(tok), tok->type == TOKEN_KW_TRUE);
}

//! Depending on the size of the constant literal, get the smallest type that would fit it
const struct type * ast_constant_get_storagetype(struct ast_node *n);

#include <ast/ast.h>

i_INLINE_DECL enum constant_type ast_constant_get_type(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT);
    return n->constant.type;
}

i_INLINE_DECL bool ast_constant_get_float(struct ast_node *n, double *v)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT);

    if (n->constant.type != CONSTANT_NUMBER_FLOAT) {
        return false;
    }
    *v = n->constant.value.floating;

    return true;
}

i_INLINE_DECL bool ast_constant_get_integer(struct ast_node *n, int64_t *v)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT);

    if (n->constant.type != CONSTANT_NUMBER_INTEGER) {
        return false;
    }
    *v = n->constant.value.integer;

    return true;
}

i_INLINE_DECL bool ast_constant_get_bool(const struct ast_node *n)
{
    RF_ASSERT(n->type == AST_CONSTANT && n->constant.type == CONSTANT_BOOLEAN,
              "Function called for invalid ast node");
    return n->constant.value.boolean;
}
#endif
