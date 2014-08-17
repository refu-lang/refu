#include <ast/fndecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_fndecl_create(struct parser_file *f,
                                   char *sp,
                                   char *ep, 
                                   struct ast_node *name)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create(AST_FUNCTION_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->fndecl.name = name;
    ret->fndecl.ret = NULL;
    rf_ilist_head_init(&ret->fndecl.args);
    return ret;
}

void ast_fndecl_add_arg(struct ast_node *n, struct ast_node *c)
{
    RF_ASSERT(n->type == AST_FUNCTION_DECLARATION);
    RF_ASSERT(c->type == AST_VARIABLE_DECLARATION);

    rf_ilist_add(&n->fndecl.args, &c->lh);
}

void ast_fndecl_set_ret(struct ast_node *n, struct ast_node *r)
{
    RF_ASSERT(n->type == AST_FUNCTION_DECLARATION);
    RF_ASSERT(r->type == AST_IDENTIFIER);
 
    n->fndecl.ret = r;
}

struct RFstring *ast_fndecl_name_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_FUNCTION_DECLARATION);

    return ast_identifier_str(n->fndecl.name);
}

struct RFstring *ast_fndecl_ret_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_FUNCTION_DECLARATION);

    return ast_identifier_str(n->fndecl.ret);
}
