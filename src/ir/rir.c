#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_block.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir_typedef.h>
#include <ir/rir_utils.h>
#include <types/type.h>
#include <types/type_operators.h>
#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>
#include <String/rf_str_common.h>
#include <String/rf_str_corex.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <ast/string_literal.h>
#include <utils/common_strings.h>
#include <analyzer/type_set.h>
#include <module.h>
#include <compiler.h>

static inline void rir_ctx_init(struct rir_ctx *ctx, struct rir *r, struct module *m)
{
    RF_STRUCT_ZERO(ctx);
    ctx->common.rir = r;
    darray_init(ctx->st_stack);
    rir_ctx_push_st(ctx, module_symbol_table(m));
}

static inline void rir_ctx_deinit(struct rir_ctx *ctx)
{
    darray_free(ctx->st_stack);
}

void rir_ctx_reset(struct rir_ctx *ctx)
{
    ctx->expression_idx = 0;
    ctx->label_idx = 0;
}

void rir_ctx_push_st(struct rir_ctx *ctx, struct symbol_table *st)
{
    darray_append(ctx->st_stack, st);
}

struct symbol_table *rir_ctx_pop_st(struct rir_ctx *ctx)
{
    RF_ASSERT(darray_size(ctx->st_stack) > 0, "Tried to pop from an empty stack");
    return darray_pop(ctx->st_stack);
}

struct symbol_table *rir_ctx_curr_st(struct rir_ctx *ctx)
{
    return darray_top(ctx->st_stack);
}

i_INLINE_INS struct rir_block *rir_ctx_curr_block(struct rir_ctx *ctx);
i_INLINE_INS struct rir_fndef *rir_ctx_curr_fn(struct rir_ctx *ctx);
i_INLINE_INS struct rir *rir_ctx_rir(struct rir_ctx *ctx);

bool rir_ctx_st_setobj(struct rir_ctx *ctx, const struct RFstring *id, struct rir_object *obj)
{
    struct symbol_table_record *rec = symbol_table_lookup_record(rir_ctx_curr_st(ctx), id, NULL);
    if (!rec) {
        return false;
    }
    rec->rirobj = obj;
    return true;
}

bool rir_ctx_st_newobj(struct rir_ctx *ctx, const struct RFstring *id, struct type *t, struct rir_object *obj)
{

    struct symbol_table_record *rec = symbol_table_record_create_from_type(rir_ctx_curr_st(ctx), id, t);
    if (!rec) {
        return false;
    }
    rec->rirobj = obj;
    if (!symbol_table_add_record(rir_ctx_curr_st(ctx), rec)) {
        return false;
    }
    return true;
}

struct rir_object *rir_ctx_st_getobj(struct rir_ctx *ctx, const struct RFstring *id)
{
    struct symbol_table_record *rec = symbol_table_lookup_record(rir_ctx_curr_st(ctx), id, NULL);
    return rec ? rec->rirobj : NULL;
}

void rir_strec_create_allocas(struct symbol_table_record *rec,
                              struct rir_ctx *ctx)
{
    struct rir_type *type = rir_type_create_from_type(symbol_table_record_type(rec), ctx);
    RF_ASSERT_OR_EXIT(type, "Could not create a rir_type during symbol table iteration");
    struct rir_object *alloca = rir_alloca_create_obj(type, rec->id, RIRPOS_AST, ctx);
    RF_ASSERT_OR_EXIT(alloca, "Could not create an alloca object during symbol table iteration");
    rec->rirobj = alloca;
}

void rir_strec_add_allocas(struct symbol_table_record *rec,
                           struct rir_ctx *ctx)
{
    RF_ASSERT(rec->rirobj && rec->rirobj->category == RIR_OBJ_EXPRESSION,
              "Expected an expression rir object");
    rir_common_block_add(&ctx->common, &rec->rirobj->expr);
}

static void rir_strec_create_and_add_allocas(struct symbol_table_record *rec,
                                             struct rir_ctx *ctx)
{
    rir_strec_create_allocas(rec, ctx);
    rir_strec_add_allocas(rec, ctx);
}

void rir_ctx_st_create_allocas(struct rir_ctx *ctx)
{
    symbol_table_iterate(rir_ctx_curr_st(ctx),
                         (htable_iter_cb)rir_strec_create_allocas,
                         ctx);
}

void rir_ctx_st_add_allocas(struct rir_ctx *ctx)
{
    symbol_table_iterate(rir_ctx_curr_st(ctx),
                         (htable_iter_cb)rir_strec_add_allocas,
                         ctx);
}

void rir_ctx_st_create_and_add_allocas(struct rir_ctx *ctx)
{
    symbol_table_iterate(rir_ctx_curr_st(ctx),
                         (htable_iter_cb)rir_strec_create_and_add_allocas,
                         ctx);
}

static struct rir_value *rir_ctx_lastobj_get(struct rir_object *obj)
{
    if (!obj) {
        return NULL;
    }
    return rir_object_value(obj);
}
struct rir_value *rir_ctx_lastval_get(const struct rir_ctx *ctx)
{
    return rir_ctx_lastobj_get(ctx->returned_obj);
}
struct rir_value *rir_ctx_lastassignval_get(const struct rir_ctx *ctx)
{
    return rir_ctx_lastobj_get(ctx->last_assign_obj);
}


static bool rir_init(struct rir *r)
{
    RF_STRUCT_ZERO(r);
    strmap_init(&r->map);
    strmap_init(&r->global_literals);
    rf_ilist_head_init(&r->functions);
    rf_ilist_head_init(&r->objects);
    rf_ilist_head_init(&r->typedefs);
    r->rir_types_pool = rf_fixed_memorypool_create(sizeof(struct rir_type),
                                                   RIR_TYPES_POOL_CHUNK_SIZE);
    if (!r->rir_types_pool) {
        return false;
    }
    darray_init(r->dependencies);
    darray_init(r->free_values);
    return true;
}

struct rir *rir_create()
{
    struct rir *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_init(ret)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_deinit(struct rir *r)
{
    struct rir_fndecl *fn;
    struct rir_fndecl *tmp;
    darray_free(r->dependencies);
    strmap_clear(&r->map);
    strmap_clear(&r->global_literals);

    rf_ilist_for_each_safe(&r->functions, fn, tmp, ln) {
        rir_function_destroy(fn);
    }
    rf_string_deinit(&r->name);

    // free all free standing rir values
    struct rir_value **val;
    darray_foreach(val, r->free_values) {
        rir_value_destroy(*val, RIR_VALUE_NORMAL);
    }
    darray_free(r->free_values);

    // all other rir objects are in the global rir object list so destroy them
    struct rir_object *obj;
    struct rir_object *tmpobj;
    rf_ilist_for_each_safe(&r->objects, obj, tmpobj, ln) {
        rir_object_destroy(obj);
    }

    if (r->buff) {
        rf_stringx_destroy(r->buff);
    }

    if (r->types_set) {
        type_objset_destroy(r->types_set, r->types_pool);
    }
    if (r->types_pool) {
        rf_fixed_memorypool_destroy(r->types_pool);
    }
    rf_fixed_memorypool_destroy(r->rir_types_pool);
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}

/* -- functions for finalizing the ast and creating the RIR -- */

static inline void rir_move_from_module(struct rir *r, struct module *m)
{
    // move types set into the rir
    r->types_set = m->types_set;
    m->types_set = NULL;
    // move types memory pool to the rir
    r->types_pool = m->types_pool;
    m->types_pool = 0;
}

static inline bool rir_create_typedefs(struct rf_objset_type *typeset,
                                       struct RFilist_head *typedefs_list,
                                       struct rir_ctx *ctx)
{
    struct type **t;
    struct type_arr tarr;
    bool ret = false;
    // ORDER MATTERS here, since types that depend on others should be done first
    if (!typeset_to_ordered_array(typeset, &tarr)) {
        RF_ERROR("Failed to create an ordered array out of a typeset");
        return false;
    }
    darray_foreach(t, tarr) {
        if (!type_is_elementary(*t) && !type_is_implop(*t)) {
            struct rir_typedef *def = rir_typedef_create_from_type(*t, ctx);
            if (!def) {
                RF_ERROR("Failed to create a RIR typedef");
                goto end;
            }
            rf_ilist_add_tail(typedefs_list,  &def->ln);
        }
    }
    ret = true;
end:
    darray_free(tarr);
    return ret;
}

static bool rir_process_do(struct rir *r, struct module *m)
{
    RF_ASSERT(module_rir_codepath(m) == RIRPOS_AST,
              "Should not come here from a RIR parsing codepath");
    bool ret = false;
    struct ast_node *child;
    struct rir_ctx ctx;

    rir_move_from_module(r, m);

    rir_ctx_init(&ctx, r, m);
    // assign the name to this rir
    if (!rf_string_copy_in(&r->name, module_name(m))) {
        RF_ERROR("Could not assign a name to a RIR module object");
        return false;
    }
    // for each of the module's dependencies, add equivalent rir dependencies
    struct module **dep;
    darray_foreach(dep, m->dependencies) {
        RF_ASSERT((*dep)->rir, "A dependency's RIR was not calculated");
        darray_append(r->dependencies, (*dep)->rir);
    }

    // add "true", "false" as global literals in the module
    if (!rir_global_addorget_string(rir_ctx_rir(&ctx), &g_str_true) ||
        !rir_global_addorget_string(rir_ctx_rir(&ctx), &g_str_false)) {
        RF_ERROR("Failed to add \"true\", \"false\" global literals in the rir strmap");
        goto end;
    }
    // for all string literals in the module create global strings to be
    // reused in case a literal is used more than once
    struct rf_objset_iter it;
    struct RFstring *s;
    rf_objset_foreach(&m->string_literals_set, &it, s) {
        if (!rir_global_addorget_string(rir_ctx_rir(&ctx), s)) {
            RF_ERROR("Failed to add a global string literal to the RIR");
            goto end;
        }
    }

    // for each non elementary, non sum-type rir type in this module and its dependencies create a typedef
    if (!rir_create_typedefs(r->types_set, &r->typedefs, &ctx)) {
        RF_ERROR("Failed to create a RIR typedef");
        goto end;
    }
    struct rir **rir_dep;
    darray_foreach(rir_dep, r->dependencies) {
        if (!rir_create_typedefs((*rir_dep)->types_set, &r->typedefs, &ctx)) {
            RF_ERROR("Failed to create a RIR typedef");
            goto end;
        }
    }

    // for each of the foreign imported functions create a rir declaration
    struct ast_node **foreigndecl;
    struct rir_fndecl *fndecl;
    darray_foreach(foreigndecl, m->foreignfn_arr) {
        fndecl = rir_fndecl_create_from_ast(*foreigndecl, &ctx);
        if (!fndecl) {
            RF_ERROR("Failed to create a RIR function declaration");
            goto end;
        }
        rf_ilist_add_tail(&r->functions, &fndecl->ln);
    }

    // for each function of the module, create a rir equivalent
    struct rir_fndef *fndef;
    rf_ilist_for_each(&m->node->children, child, lh) {
        if (child->type == AST_FUNCTION_IMPLEMENTATION) {
            fndef = rir_fndef_create_from_ast(child, &ctx);
            if (!fndef) {
                RF_ERROR("Failed to create a RIR function definition");
                goto end;
            }
            rf_ilist_add_tail(&r->functions, &fndef->decl.ln);
        }
    }

    // success
    ret = true;
end:
    rir_ctx_deinit(&ctx);
    return ret;
}

bool compiler_create_rir()
{
    struct compiler *c = compiler_instance_get();
    // create some utilities needed by all the rir modules. Freed at compiler_deinit.
    if (!rir_utils_create()) {
        return false;
    }
    // for each module of the compiler that needs it, process and create the rir
    struct module *mod;
    rf_ilist_for_each(&c->sorted_modules, mod, ln) {
        if (module_rir_codepath(mod) == RIRPOS_AST) {
            if (!rir_process_do(mod->rir, mod)) {
                RF_ERROR("Failed to create the RIR for module \""RF_STR_PF_FMT"\"",
                         module_name(mod));
                return false;
            }
        }
    }
    return true;
}

void rirtostr_ctx_reset(struct rirtostr_ctx *ctx)
{
    darray_clear(ctx->visited_blocks);
    darray_free(ctx->visited_blocks);
    darray_init(ctx->visited_blocks);
}

static inline void rirtostr_ctx_deinit(struct rirtostr_ctx *ctx)
{
    darray_free(ctx->visited_blocks);
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

static bool itprint_literals_cb(const struct RFstring *member, struct rir_object *obj, struct rirtostr_ctx *ctx)
{
    if (!rir_global_tostring(ctx, &obj->global)) {
        RF_ERROR("Failed to turn a rir global to a string");
        return false;
    }
    return true;
}

struct RFstring *rir_tostring(struct rir *r)
{
    struct RFstring *ret = NULL;
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

    // output global string literals
    strmap_iterate(&r->global_literals, (strmap_it_cb)itprint_literals_cb, &ctx);

    // output typedefinitions
    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (!rir_typedef_tostring(&ctx, def)) {
            RF_ERROR("Failed to turn a rir typedef to a string");
            goto end;
        }
    }

    // output functions
    struct rir_fndecl *decl;
    rf_ilist_for_each(&r->functions, decl, ln) {
        if (!rir_function_tostring(&ctx, decl)) {
            RF_ERROR("Failed to turn a rir function "RF_STR_PF_FMT" to a string",
                     RF_STR_PF_ARG(&decl->name));
            goto end;
        }
    }
    // success
    ret = RF_STRX2STR(r->buff);

end:
    rirtostr_ctx_deinit(&ctx);
    return ret;
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

struct rir_fndecl *rir_fndecl_byname(const struct rir *r, const struct RFstring *name)
{
    struct rir_fndecl *fn;
    rf_ilist_for_each(&r->functions, fn, ln) {
        if (rf_string_equal(name, &fn->name)) {
            return fn;
        }
    }

    // if not found here search in the first of the dependencies which should be the stdlib
    if (darray_size(r->dependencies) != 0) {
        struct rir *dep = darray_item(r->dependencies, 0);
        RF_ASSERT(rf_string_equal(&dep->name, &g_str_stdlib),
        "The first dependency should be the standard library");
        rf_ilist_for_each(&dep->functions, fn, ln) {
            if (rf_string_equal(name, &fn->name)) {
                return fn;
            }
        }
    }
    return NULL;
}

struct rir_typedef *rir_typedef_frommap(const struct rir *r, const struct RFstring *name)
{
    struct rir_object *obj = strmap_get(&r->map, name);
    if (!obj) {
        return NULL;
    }
    return rir_object_get_typedef(obj);
}

struct rir_typedef *rir_typedef_byname(const struct rir *r, const struct RFstring *name)
{
    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (rf_string_equal(name, &def->name)) {
            return def;
        }
    }
    return NULL;
}

struct rir_type *rir_type_byname(struct rir *r, const struct RFstring *name)
{
    const struct rir_type *type = rir_type_elem_get_from_string(name, false);
    if (type) {
        return (struct rir_type*)type;
    }
    // not elementary, search for typedef
    struct rir_typedef *def = rir_typedef_byname(r, name);
    if (!def) {
        return NULL;
    }
    return rir_type_comp_create(def, r, false);
}

struct rir_object *rir_strlit_obj(const struct rir *r, const struct ast_node *n)
{
    return strmap_get(&r->global_literals, ast_string_literal_get_str(n));
}

void rir_freevalues_add(struct rir *r, struct rir_value *v)
{
    darray_append(r->free_values, v);
}
