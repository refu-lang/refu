#ifndef LFR_AST_STRING_LITERAL_H
#define LFR_AST_STRING_LITERAL_H

#include <stdbool.h>

#include <rflib/string/rf_str_decl.h>
#include <rflib/defs/inline.h>

struct ast_node;
struct inplocation;
struct module;

struct ast_node *ast_string_literal_create(struct inplocation *loc);
bool ast_string_literal_hash_create(struct ast_node *lit, struct module *m);

#include <ast/ast.h>
i_INLINE_DECL const struct RFstring *
ast_string_literal_get_str(const struct ast_node *lit)
{
    AST_NODE_ASSERT_TYPE(lit, AST_STRING_LITERAL);
    return &lit->string_literal.string;
}

i_INLINE_DECL uint32_t ast_string_literal_get_hash(const struct ast_node *lit)
{
    AST_NODE_ASSERT_TYPE(lit, AST_STRING_LITERAL);
    return lit->string_literal.hash;
}

#endif
