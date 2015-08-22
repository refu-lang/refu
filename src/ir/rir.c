#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_block.h>
#include <ir/rir_type.h>
#include <ir/rir_expression.h>
#include <ir/rir_types_list.h>
#include <ir/rir_typedef.h>
#include <Utils/memory.h>
#include <String/rf_str_common.h>
#include <String/rf_str_corex.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <module.h>
#include <compiler.h>

static inline void rir_ctx_init(struct rir_ctx *ctx, struct rir *r)
{
    RF_STRUCT_ZERO(ctx);
    ctx->rir = r;
}

static bool rir_init(struct rir *r, struct module *m)
{
    RF_STRUCT_ZERO(r);
    rf_ilist_head_init(&r->functions);
    rf_ilist_head_init(&r->typedefs);
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
    struct rir_typedef *def;
    struct rir_typedef *deftmp;
    rf_ilist_for_each_safe(&r->typedefs, def, deftmp, ln) {
        rir_typedef_destroy(def);
    }
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}

/* -- functions for finalizing the ast and creating the RIR -- */

static bool rir_process_do(struct rir *r, struct module *m)
{
    struct ast_node *child;
    struct rir_fndecl *fndecl;
    struct rir_ctx ctx;
    struct rir_type *t;

    rir_ctx_init(&ctx, r);

    // for each rir type create a typedef/uniondef
    rir_types_list_for_each(r->rir_types_list, t) {
        if (!rir_type_is_elementary(t)) {
            struct rir_typedef *def = rir_typedef_create(t);
            rf_ilist_add_tail(&r->typedefs,  &def->ln);
        }
    }

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

bool rir_process(struct compiler *c)
{
    // for each module of the compiler do rir to string
    struct module *mod;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        if (!rir_process_do(mod->rir, mod)) {
            return false;
        }
    }
    return true;
}


struct RFstring *rir_tostring(struct rir *r)
{
    if (r->buff) {
        return RF_STRX2STR(r->buff);
    }

    r->buff = rf_stringx_create_buff(1024, "");
    if (!r->buff) {
        RF_ERROR("Failed to create the string buffer for rir outputting");
        return NULL;
    }

    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (!rir_typedef_tostring(r, def)) {
            RF_ERROR("Failed to turn a rir typedef to a string");
            return NULL;
        }
    }

    struct rir_fndecl *fn;
    rf_ilist_for_each(&r->functions, fn, ln) {
        if (!rir_fndecl_tostring(r, fn)) {
            RF_ERROR("Failed to turn a rir function "RF_STR_PF_FMT" to a string",
                     RF_STR_PF_ARG(fn->name));
            return NULL;
        }
    }

    return RF_STRX2STR(r->buff);
}

bool rir_print(struct compiler *c)
{
    // for each module of the compiler do rir to string
    struct module *mod;
    struct RFstring *s;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        if (!(s = rir_tostring(mod->rir))) {
            return false;
        }
        printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(s));
    }
    return true;
}


void rirctx_block_add(struct rir_ctx *ctx, struct rir_expression *expr)
{
    rf_ilist_add_tail(&ctx->current_block->expressions, &expr->ln);
}
