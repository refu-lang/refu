#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_types_list.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <module.h>

static bool rir_init(struct rir *r, struct module *m)
{
    RF_STRUCT_ZERO(r);
    rf_ilist_head_init(&r->functions);
    // create the rir types list from the types set for this module
    if (!(r->rir_types_list = rir_types_list_create(m->types_set))) {
        return false;
    }
    return true;
}

struct rir *rir_create(struct module *m)
{
    struct rir *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_init(ret, m)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_deinit(struct rir *r)
{
    if (r->rir_types_list) {
        rir_types_list_destroy(r->rir_types_list);
    }
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}

/* -- functions for finalizing the ast and creating the RIR -- */

struct rir_traversal_ctx {
    struct module *mod;
    struct rir *rir;
};

static inline void rir_traversal_ctx_init(struct rir_traversal_ctx *ctx,
                                          struct rir *r,
                                          struct module *m)
{
    ctx->mod = m;
    ctx->rir = r;
}


bool rir_fndecl_add(struct ast_node *n, struct rir_traversal_ctx *ctx)
{
    struct rir_fndecl *fndecl = rir_fndecl_create(n);
    if (!fndecl) {
        return false;
    }
    rf_ilist_add(&ctx->rir->functions, &fndecl->ln);
    return true;
}

static bool rir_ast_finalize_do(struct ast_node *n, void *user_arg)
{
    struct rir_traversal_ctx *ctx = user_arg;
    switch(n->type) {
    case AST_FUNCTION_IMPLEMENTATION:
        if (!rir_fndecl_add(n, ctx)) {
            return false;
        }
        break;
    default: // ignore all others
        break;
    }
    return true;
}

bool rir_ast_finalize(struct rir *r, struct module *m)
{
    struct rir_traversal_ctx ctx;
    rir_traversal_ctx_init(&ctx, r, m);
    return ast_pre_traverse_tree(m->node, rir_ast_finalize_do, &ctx);
}
