#include <ast/function.h>

#include <ast/operators.h>

/* -- function declaration functions -- */

struct ast_node *ast_fndecl_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   enum fndecl_position pos,
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

    ret->fndecl.position = pos;
    ast_node_register_child(ret, name, fndecl.name);
    ast_node_register_child(ret, genr, fndecl.genr);
    ast_node_register_child(ret, args, fndecl.args);
    ast_node_register_child(ret, ret_type, fndecl.ret);

    return ret;
}

i_INLINE_INS const struct RFstring *ast_fndecl_name_str(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_fndecl_genrdecl_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_fndecl_args_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_fndecl_return_get(struct ast_node *n);
i_INLINE_INS enum fndecl_position ast_fndecl_position_get(struct ast_node *n);
i_INLINE_INS bool ast_fndecl_symbol_table_init(struct ast_node *n,
                                               struct analyzer *a);
i_INLINE_INS struct symbol_table *ast_fndecl_symbol_table_get(struct ast_node *n);

/* -- function implementation functions -- */

struct ast_node *ast_fnimpl_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
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
    ret->fnimpl.st = NULL;

    return ret;
}

i_INLINE_INS struct ast_node *ast_fnimpl_fndecl_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_fnimpl_body_get(struct ast_node *n);
i_INLINE_INS void ast_fnimpl_symbol_table_set(struct ast_node *n,
                                              struct symbol_table *st);
i_INLINE_INS struct symbol_table *ast_fnimpl_symbol_table_get(struct ast_node *n);

/* -- function call functions -- */

struct ast_node *ast_fncall_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *args,
                                   struct ast_node *genr)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(name, AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_FUNCTION_CALL, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->fncall.params_type = NULL;
    ast_node_register_child(ret, name, fncall.name);
    ast_node_register_child(ret, args, fncall.args);
    ast_node_register_child(ret, genr, fncall.genr);

    return ret;
}

i_INLINE_INS const struct RFstring* ast_fncall_name(struct ast_node *n);
i_INLINE_INS struct ast_node* ast_fncall_args(struct ast_node *n);
i_INLINE_INS struct ast_node* ast_fncall_genr(struct ast_node *n);
i_INLINE_INS const struct type *ast_fncall_params_type(struct ast_node *n);

static bool do_ast_fncall_for_each_arg(struct ast_node *arg, fncall_args_cb cb, void *user_arg)
{
    if (arg->type == AST_BINARY_OPERATOR) {
        if (!do_ast_fncall_for_each_arg(ast_binaryop_left(arg), cb, user_arg)) {
            return false;
        }
        if (!do_ast_fncall_for_each_arg(ast_binaryop_right(arg), cb, user_arg)) {
            return false;
        }
        return true;
    }
    return cb(arg, user_arg);
}

bool ast_fncall_for_each_arg(struct ast_node *n, fncall_args_cb cb, void *user_arg)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    struct ast_node *args = ast_fncall_args(n);
    if (!args) {
        return true;
    }
    return do_ast_fncall_for_each_arg(args, cb, user_arg);
}
