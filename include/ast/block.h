#ifndef LFR_AST_BLOCK_H
#define LFR_AST_BLOCK_H

#include <ast/ast.h>
struct ast_node *ast_block_create();

i_INLINE_DECL struct symbol_table* ast_block_get_symbol_table(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    return &n->block.st;
}
#endif
