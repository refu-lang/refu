#include <analyzer/analyzer.h>

#include <module.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ast/ast_utils.h>
#include <types/type.h>
#include <ir/rir.h>


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
                              
static void analyzer_finalize_fndecl(struct ast_node *n)
{
    // figure out the number of arguments
    struct ast_node *fn_args = ast_fndecl_args_get(n);
    if (fn_args) {
        // TODO: with type_creation.c:type_function_init_cb() we create an
        //       arguments array and as such we can simply cound the size there.
        //       just move the counting there and drop this function?
        const struct type *t = ast_node_get_type(ast_fndecl_args_get(n));
        n->fndecl.args_num = (darray_size(t->operator.operands) == 0)
            ? 1
            : darray_size(t->operator.operands);
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
    m->rir = rir_create();
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
