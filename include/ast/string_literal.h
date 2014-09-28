#ifndef LFR_AST_STRING_LITERAL_H
#define LFR_AST_STRING_LITERAL_H

#include <String/rf_str_decl.h>
#include <Definitions/inline.h>

struct ast_node;
struct inplocation;

struct ast_node *ast_string_literal_create(struct inplocation *loc);

#include <ast/ast.h>
i_INLINE_DECL const struct RFstring *
ast_string_literal_get_str(struct ast_node *lit)
{
    return &lit->string_literal.string;
}

#endif
