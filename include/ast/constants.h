#ifndef LFR_AST_CONSTANTS_H
#define LFR_AST_CONSTANTS_H

#include <stdbool.h>
#include <stdint.h>

#include <rflib/defs/inline.h>

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

/**
 * Get a string representation of the constant
 *
 * Since this creates a temporary string, the call needs to be enclosed in
 * RFS_PUSH() and RFS_POP()
 */
const struct RFstring *ast_constant_string(const struct ast_constant *c);

const struct RFstring *ast_constanttype_string(enum constant_type type);
i_INLINE_DECL const struct RFstring *ast_constant_type_string(const struct ast_constant *c)
{
    return ast_constanttype_string(c->type);
}

//! Depending on the size of the constant literal, get the smallest type that would fit it
const struct type *ast_constant_get_storagetype(struct ast_node *n);

#include <ast/ast.h>

i_INLINE_DECL void ast_constant_init_int(struct ast_constant *c, int64_t n)
{
    c->type = CONSTANT_NUMBER_INTEGER;
    c->value.integer = n;
}

i_INLINE_DECL void ast_constant_init_float(struct ast_constant *c, double n)
{
    c->type = CONSTANT_NUMBER_FLOAT;
    c->value.floating = n;
}

i_INLINE_DECL void ast_constant_init_bool(struct ast_constant *c, bool n)
{
    c->type = CONSTANT_BOOLEAN;
    c->value.boolean = n;
}

i_INLINE_DECL enum constant_type ast_constant_get_type(const struct ast_constant *n)
{
    return n->type;
}

i_INLINE_DECL bool ast_constant_get_float(const struct ast_constant *n, double *v)
{
    if (n->type != CONSTANT_NUMBER_FLOAT) {
        return false;
    }
    *v = n->value.floating;
    return true;
}

i_INLINE_DECL bool ast_constant_get_integer(const struct ast_constant *n, int64_t *v)
{
    if (n->type != CONSTANT_NUMBER_INTEGER) {
        return false;
    }
    *v = n->value.integer;
    return true;
}

i_INLINE_DECL bool ast_constant_get_bool(const struct ast_constant *n)
{
    RF_ASSERT(n->type == CONSTANT_BOOLEAN,
              "Function called for invalid ast node");
    return n->value.boolean;
}
#endif
