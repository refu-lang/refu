#include <ast/module.h>
#include <ast/ast.h>
/* -- ast import functions -- */
struct ast_node *ast_import_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   bool foreign)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_IMPORT, start, end);
    if (!ret) {
        return NULL;
    }
    ret->import.foreign = foreign;
    return ret;
}

i_INLINE_INS bool ast_import_is_foreign(const struct ast_node *n);

/* -- ast module functions -- */
struct ast_node *ast_module_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *args)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_MODULE, start, end);
    if (!ret) {
        return NULL;
    }
    ast_node_register_child(ret, name, module.name);
    ast_node_register_child(ret, args, module.args);

    return ret;
}

i_INLINE_INS struct symbol_table *ast_module_symbol_table_get(struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_module_name(const struct ast_node *n);
