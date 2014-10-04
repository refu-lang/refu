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
    return ret;
}
