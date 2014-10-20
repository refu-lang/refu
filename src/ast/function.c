#include <ast/function.h>


struct ast_node *ast_fndecl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *genr,
                                   struct ast_node *args,
                                   struct ast_node *ret_type)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_FUNCTION_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, fndecl.name);
    ast_node_register_child(ret, genr, fndecl.genr);
    ast_node_register_child(ret, args, fndecl.args);
    ast_node_register_child(ret, ret_type, fndecl.ret);

    return ret;
}

i_INLINE_INS const struct RFstring *ast_fndecl_name_str(const struct ast_node *n);
i_INLINE_INS bool ast_fndecl_symbol_table_init(struct ast_node *n,
                                               struct analyzer *a);
i_INLINE_INS struct symbol_table *ast_fndecl_symbol_table_get(struct ast_node *n);

struct ast_node *ast_fnimpl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *decl,
                                   struct ast_node *body)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(decl, AST_FUNCTION_DECLARATION);
    AST_NODE_ASSERT_TYPE(body, AST_BLOCK);

    ret = ast_node_create_marks(AST_FUNCTION_IMPLEMENTATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, decl, fnimpl.decl);
    ast_node_register_child(ret, body, fnimpl.body);

    return ret;
}

struct ast_node *ast_fncall_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *genr)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_FUNCTION_CALL, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, fncall.name);
    ast_node_register_child(ret, genr, fncall.genr);

    return ret;
}
