#include <ast/datadecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_datadecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create(AST_DATA_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    rf_ilist_head_init(&ret->datadecl.members);
    ret->datadecl.name = name;
    return ret;
}

void ast_datadecl_destroy(struct ast_node *n)
{
    struct ast_node *m;
    struct ast_node *tmp;

    ast_node_destroy(n->datadecl.name);
    rf_ilist_for_each_safe(&n->datadecl.members, m, tmp, lh) {
        ast_node_destroy(m);
    }
}

void ast_datadecl_add_member(struct ast_node *n, struct ast_node *c)
{
    RF_ASSERT(n->type == AST_DATA_DECLARATION);
    RF_ASSERT(c->type == AST_VARIABLE_DECLARATION);

    rf_ilist_add(&n->datadecl.members, &c->lh);
}


struct RFstring *ast_datadecl_name_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_DATA_DECLARATION);

    return ast_identifier_str(n->datadecl.name);
}
