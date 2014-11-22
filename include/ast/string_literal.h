#ifndef LFR_AST_STRING_LITERAL_H
#define LFR_AST_STRING_LITERAL_H

#include <stdbool.h>

#include <String/rf_str_decl.h>
#include <Definitions/inline.h>

struct ast_node;
struct inplocation;
struct analyzer;

struct ast_node *ast_string_literal_create(struct inplocation *loc);
bool ast_string_literal_hash_create(struct ast_node *lit, struct analyzer *a);

const struct RFstring *ast_string_literal_analyzed_str(const struct ast_node *n,
                                                       const struct analyzer *a);

#include <ast/ast.h>
i_INLINE_DECL const struct RFstring *
ast_string_literal_get_str(struct ast_node *lit)
{
    return &lit->string_literal.string;
}

#endif
