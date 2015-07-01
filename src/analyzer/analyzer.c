#include <analyzer/analyzer.h>

#include "global_context.h"

#include <Utils/memory.h>

#include <module.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ast/module.h>
#include <ast/ast_utils.h>
#include <parser/parser.h>
#include <types/type.h>
#include <analyzer/type_set.h>


i_INLINE_INS void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
                                              struct module *m);
i_INLINE_INS void analyzer_traversal_ctx_deinit(struct analyzer_traversal_ctx *ctx);
i_INLINE_INS struct ast_node *analyzer_traversal_ctx_get_nth_parent(
    unsigned int num,
    struct analyzer_traversal_ctx *ctx);
i_INLINE_INS struct ast_node *analyzer_traversal_ctx_get_nth_parent_or_die(
    unsigned int num,
    struct analyzer_traversal_ctx *ctx);

bool analyzer_traversal_ctx_traverse_parents(struct analyzer_traversal_ctx *ctx,
                                             analyzer_traversal_parents_cb cb,
                                             void *user_arg)
{
    int index = darray_size(ctx->parent_nodes) - 2;
    while (index >= 0) {
        if (cb(darray_item(ctx->parent_nodes, index), user_arg)) {
            return true;
        }
        --index;
    }
    return false;
}


static bool module_determine_dependencies_do(struct ast_node *n, void *user_arg)
{
    struct module *mod = user_arg;
    switch (n->type) {
    case AST_IMPORT:
        if (!ast_import_is_foreign(n)) {
            return module_add_import(mod, n);
        }
    default:
        break;
    }
    return true;
}

bool module_determine_dependencies(struct module *m, bool use_stdlib)
{
    // initialize module symbol table here instead of analyzer_first_pass
    // since we need it beforehand to get symbols from import
    if (!module_symbol_table_init(m)) {
        RF_ERROR("Could not initialize symbol table for root node");
        return false;
    }

    // read the imports and add dependencies
    if (!ast_pre_traverse_tree(m->node, module_determine_dependencies_do, m)) {
        return false;
    }

    // TODO: This can't be the best way to achieve this. Rethink when possible
    // if this is the main module add the stdlib as dependency,
    // unless a program without the stdlib was requested
    if (use_stdlib && module_is_main(m)) {
        return module_add_stdlib(m);
    }
    return true;
}
                              

static void analyzer_finalize_fndecl(struct ast_node *n)
{
    // figure out the number of arguments
    struct ast_node *fn_args = ast_fndecl_args_get(n);
    if (fn_args) {
        const struct rir_type *rtype = type_get_rir_or_die(ast_node_get_type(ast_fndecl_args_get(n), AST_TYPERETR_AS_LEAF));
        n->fndecl.args_num = (darray_size(rtype->subtypes) == 0) ? 1 : darray_size(rtype->subtypes);
    } else {
        n->fndecl.args_num = 0;
    }
}

static enum traversal_cb_res analyzer_finalize_do(struct ast_node *n, void *user_arg)
{
    struct module *m = user_arg;
    (void)m;
    switch (n->type) {
    case AST_FUNCTION_DECLARATION:
        analyzer_finalize_fndecl(n);
        break;
    default:
        break;
    }
    // finally set the state
    n->state = AST_NODE_STATE_RIR_END;
    return TRAVERSAL_CB_OK;
}

static bool do_nothing(struct ast_node *n, void *user_arg) { return true; }

bool analyzer_finalize(struct module *m)
{
    // for now copy types of all dependencies.
    // TODO: Should be searching the dependencies when needed and not copy here
    struct module **dependency;
    darray_foreach(dependency, m->dependencies) {
        struct rf_objset_iter it;
        struct type *t;
        rf_objset_foreach((*dependency)->types_set, &it, t) {
            if (!rf_objset_add(m->types_set, type, t)) {
                RF_ERROR("rf_objset_add() failure");
                return false;
            }
        }
    }

    RF_ASSERT(!m->rir_types_list, "An analyzer's rir type list should not have been created before this point");
    // create the rir types list from the types set for this module
    if (!(m->rir_types_list = rir_types_list_create(m->types_set))) {
        return false;
    }
    // TODO: if we don't have any actual pre_callback then use ast_post_traverse_tree()
    bool ret = (TRAVERSAL_CB_OK == ast_traverse_tree_nostop_post_cb(
                    m->node,
                    do_nothing,
                    NULL,
                    analyzer_finalize_do,
                    m
                )
    );
    return ret;    
}
