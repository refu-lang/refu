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

void rir_ctx_reset(struct rir_ctx *ctx)
{
    ctx->expression_idx = 0;
    ctx->label_idx = 0;
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

    // for each non elementary rir type create a typedef/uniondef
    rir_types_list_for_each(r->rir_types_list, t) {
        if (!rir_type_is_elementary(t) && t->category != COMPOSITE_IMPLICATION_RIR_TYPE ) {
            struct rir_typedef *def = rir_typedef_create(t);
            rf_ilist_add_tail(&r->typedefs,  &def->ln);
#if 0
            // TEMP TO SEE what types are created
            printf(RF_STR_PF_FMT"", RF_STR_PF_ARG(rir_type_str_or_die(t)));
            if (t->name) {
                printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(t->name));
            } else {
                printf("\n");
            }
#endif
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

void rirtostr_ctx_reset(struct rirtostr_ctx *ctx)
{
    darray_clear(ctx->visited_blocks);
    darray_init(ctx->visited_blocks);
}

static inline void rirtostr_ctx_deinit(struct rirtostr_ctx *ctx)
{
    darray_clear(ctx->visited_blocks);
}

void rirtostr_ctx_init(struct rirtostr_ctx *ctx, struct rir *r)
{
    ctx->rir = r;
    darray_init(ctx->visited_blocks);
}

void rirtostr_ctx_visit_block(struct rirtostr_ctx *ctx, const struct rir_block *b)
{
    darray_append(ctx->visited_blocks, b);
}

bool rirtostr_ctx_block_visited(struct rirtostr_ctx *ctx, const struct rir_block *b)
{
    const struct rir_block **block;
    darray_foreach(block, ctx->visited_blocks) {
        if (*block == b) {
            return true;
        }
    }
    return false;
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

    struct rirtostr_ctx ctx;
    rirtostr_ctx_init(&ctx, r);
    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (!rir_typedef_tostring(&ctx, def)) {
            RF_ERROR("Failed to turn a rir typedef to a string");
            goto fail_free_ctx;
        }
    }

    struct rir_fndecl *fn;
    rf_ilist_for_each(&r->functions, fn, ln) {
        if (!rir_fndecl_tostring(&ctx, fn)) {
            RF_ERROR("Failed to turn a rir function "RF_STR_PF_FMT" to a string",
                     RF_STR_PF_ARG(fn->name));
            goto fail_free_ctx;
        }
    }
    return RF_STRX2STR(r->buff);

fail_free_ctx:
    rirtostr_ctx_deinit(&ctx);
    return NULL;
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

struct rir_typedef *rir_typedef_byname(const struct rir *r, const struct RFstring *name)
{
    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (rf_string_equal(name, def->name)) {
            return def;
        }
    }
    return NULL;
}

struct rir_ltype *rir_type_byname(const struct rir *r, const struct RFstring *name)
{
    struct rir_ltype *type = rir_ltype_elem_create_from_string(name);
    if (type) {
        return type;
    }
    // not elementary, search for typedef
    struct rir_typedef *def = rir_typedef_byname(r, name);
    if (!def) {
        return NULL;
    }
    return rir_ltype_comp_create(def);
}

void rirctx_block_add(struct rir_ctx *ctx, struct rir_expression *expr)
{
    rf_ilist_add_tail(&ctx->current_block->expressions, &expr->ln);
}

