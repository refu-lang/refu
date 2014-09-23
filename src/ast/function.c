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
        //TODO: memory error
        return NULL;
    }


    ast_node_register_child(ret, name, fndecl.name);
    ast_node_register_child(ret, genr, fndecl.genr);
    ast_node_register_child(ret, args, fndecl.args);
    ast_node_register_child(ret, ret_type, fndecl.ret);

    return ret;
}

i_INLINE_INS struct RFstring *ast_fndecl_name_str(struct ast_node *n);
