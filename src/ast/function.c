#include <ast/function.h>


struct ast_node *ast_fndecl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *genr,
                                   struct ast_node *args,
                                   struct ast_node *ret_type)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

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

i_INLINE_INS struct RFstring *ast_fndecl_name_str(struct ast_node *n);


struct ast_node *ast_fnimpl_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *decl,
                                   struct ast_node *body)
{
    struct ast_node *ret;
    RF_ASSERT(decl->type == AST_FUNCTION_DECLARATION);
    RF_ASSERT(body->type == AST_BLOCK);

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
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_FUNCTION_CALL, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, fncall.name);
    ast_node_register_child(ret, genr, fncall.genr);

    return ret;
}
