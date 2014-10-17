#ifndef LFR_AST_FUNCTIONS_H
#define LFR_AST_FUNCTIONS_H

#include <ast/ast.h>
#include <Utils/sanity.h>
#include <ast/identifier.h>

#include <analyzer/symbol_table.h>

struct ast_node *ast_fndecl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *genr,
                                   struct ast_node *args,
                                   struct ast_node *ret);


i_INLINE_DECL const struct RFstring *ast_fndecl_name_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return ast_identifier_str(n->fndecl.name);
}

i_INLINE_DECL void ast_fndecl_set_symbol_table(struct ast_node *n,
                                               struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    n->fndecl.st = st;
}

struct RFstring *ast_fndecl_ret_str(struct ast_node *n);


struct ast_node *ast_fnimpl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *decl,
                                   struct ast_node *body);


struct ast_node *ast_fncall_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *genr);
#endif
