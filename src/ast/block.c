#include <ast/block.h>

#include <ast/ast.h>

struct ast_node *ast_block_create()
{
    struct ast_node *ret;
    ret = ast_node_create(AST_BLOCK);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    if (!symbol_table_init(&ret->block.st)) {
        free(ret);
        RF_ERROR("Could not initialize symbol table for a block ast node");
        return NULL;
    }

    return ret;
}

i_INLINE_INS struct symbol_table* ast_block_get_symbol_table(struct ast_node *n);
