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

    ast_block_valueexpr_set(ret, NULL);

    return ret;
}

i_INLINE_INS bool ast_block_symbol_table_init(struct ast_node *n,
                                              struct analyzer *a);
i_INLINE_INS struct symbol_table* ast_block_symbol_table_get(struct ast_node *n);
i_INLINE_INS struct ast_node* ast_block_valueexpr_get(struct ast_node *n);
i_INLINE_INS void ast_block_valueexpr_set(struct ast_node *n, struct ast_node *expr);
i_INLINE_INS void ast_block_add_element(struct ast_node *n, struct ast_node *element);
