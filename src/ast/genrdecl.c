#include <ast/genrdecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

static struct ast_genrtype *ast_genrtype_create(enum genrtype type,
                                                struct ast_node *c)
{
    struct ast_genrtype *t;
    RF_MALLOC(t, sizeof(*t), NULL);
    t->type = type;
    t->id = c;
    return t;
}

static void ast_genrtype_destroy(struct ast_genrtype *t)
{
    ast_node_destroy(t->id);
    free(t);
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
    struct ast_genrtype *m;
    struct ast_genrtype *tmp;
    rf_ilist_for_each_safe(&n->genrdecl.members, m, tmp, lh) {
        ast_genrtype_destroy(m);
    }
}

bool ast_genrdecl_add_member(struct ast_node *n,
                             enum genrtype type,
                             struct ast_node *c)
{
    struct ast_genrtype *t;
    RF_ASSERT(n->type == AST_GENERIC_DECLARATION);
    RF_ASSERT(c->type == AST_IDENTIFIER);

    t = ast_genrtype_create(type, c);
    if (!t) {
        return false;
    }

    rf_ilist_add(&n->genrdecl.members, &t->lh);
    return true;
}


