#include <ast/genrdecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_genrtype_create(struct parser_file *f, char *sp, char *ep,
                                     enum genrtype type,
                                     struct ast_node *identifier)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_GENERIC_TYPE, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->genrtype.type = type;
    ret->genrtype.id = identifier;
    return ret;
}

void ast_genrtype_destroy(struct ast_node *n)
{
    ast_node_destroy(n->genrtype.id);
}

void ast_genrtype_print(struct ast_node *n, int depth)
{
    ast_print(n->genrtype.id, depth + 1, 0);
}


struct ast_node *ast_genrdecl_create(struct parser_file *f, char *sp, char *ep)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_GENERIC_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    rf_ilist_head_init(&ret->genrdecl.members);
    return ret;
}

void ast_genrdecl_destroy(struct ast_node *n)
{
    struct ast_node *m;
    struct ast_node *tmp;
    rf_ilist_for_each_safe(&n->genrdecl.members, m, tmp, lh) {
        ast_node_destroy(m);
    }
}

void ast_genrdecl_add_member(struct ast_node *n,
                             struct ast_node *c)
{
    RF_ASSERT(n->type == AST_GENERIC_DECLARATION);
    RF_ASSERT(c->type == AST_GENERIC_TYPE);

    rf_ilist_add_tail(&n->genrdecl.members, &c->lh);
}

void ast_genrdecl_print(struct ast_node *n, int depth)
{
    struct ast_node *c;
    rf_ilist_for_each(&n->genrdecl.members, c, lh) {
        ast_print(c, depth + 1, 0);
    }
}

