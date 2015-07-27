#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_types_list.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <module.h>

static inline void rir_ctx_init(struct rir_ctx *ctx, struct rir *r)
{
    ctx->rir = r;
    ctx->current_fn = 0;
}

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
    struct rir_fndecl *fn;
    struct rir_fndecl *tmp;
    if (r->rir_types_list) {
        rir_types_list_destroy(r->rir_types_list);
    }
    rf_ilist_for_each_safe(&r->functions, fn, tmp, ln) {
        rir_fndecl_destroy(fn);
    }
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}

/* -- functions for finalizing the ast and creating the RIR -- */

bool rir_ast_finalize(struct rir *r, struct module *m)
{
    struct ast_node *child;
    struct rir_fndecl *fndecl;
    struct rir_ctx ctx;
    rir_ctx_init(&ctx, r);
    // for each function of the module, create a rir equivalent
    rf_ilist_for_each(&m->node->children, child, lh) {
        if (child->type == AST_FUNCTION_IMPLEMENTATION) {
            fndecl = rir_fndecl_create(child, &ctx);
            if (!fndecl) {
                return false;
            }
            rf_ilist_add(&r->functions, &fndecl->ln);
        }
    }
    return true;
}
