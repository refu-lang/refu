#ifndef LFR_AST_MODULE_H
#define LFR_AST_MODULE_H

#include <rflib/defs/inline.h>
#include <rflib/utils/sanity.h>

#include <ast/ast.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;
/* -- ast import functions -- */
struct ast_node *ast_import_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   bool foreign);

i_INLINE_DECL bool ast_import_is_foreign(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IMPORT);
    return n->import.foreign;
}

i_INLINE_DECL bool ast_node_is_foreign_import(const struct ast_node *n)
{

    return n->type == AST_IMPORT && n->import.foreign;
}

/* -- ast module functions -- */
struct ast_node *ast_module_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *args);

i_INLINE_DECL struct symbol_table *ast_module_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MODULE);
    return &n->module.st;
}

i_INLINE_DECL const struct RFstring *ast_module_name(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_MODULE);
    return ast_identifier_str(n->module.name);
}
#endif
