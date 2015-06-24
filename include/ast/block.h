#ifndef LFR_AST_BLOCK_H
#define LFR_AST_BLOCK_H

#include <ast/ast.h>
struct ast_node *ast_block_create();

struct module;

i_INLINE_DECL bool ast_block_symbol_table_init(struct ast_node *n,
                                               struct module *m)
{
    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    return symbol_table_init(&n->block.st, m);
}

i_INLINE_DECL struct symbol_table* ast_block_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    return &n->block.st;
}

i_INLINE_DECL void ast_block_add_element(struct ast_node *n, struct ast_node *element)
{
    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    ast_node_add_child(n, element);
}
#endif
