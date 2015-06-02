#include <ir/rir.h>

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <ast/ast_utils.h>
#include <ast/function.h>
#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>
#include <types/type.h>
#include <ir/rir_type.h>
#include <ir/rir_types_list.h>



bool rir_init(struct rir *r, struct analyzer *a)
{
    // transfer ownership of memory pools
    r->symbol_table_records_pool = a->symbol_table_records_pool;
    a->symbol_table_records_pool = NULL;
    r->types_pool = a->types_pool;
    a->types_pool = NULL;

    // transfer ownership of string tables
    r->identifiers_table = a->identifiers_table;
    a->identifiers_table = NULL;
    r->string_literals_table = a->string_literals_table;
    a->string_literals_table = NULL;
    // transfer ownership of AST
    r->root = a->root;
    a->root = NULL;

    rir_types_list_init(&r->rir_types_list);

    // TODO: Probably rir types should also be a set. Just create a set out
    //       of the list for the moment, but change it later
    r->types_set = a->types_set;
    a->types_set = NULL;
    rir_types_list_populate(&r->rir_types_list, r->types_set);
    return rir_finalize_ast(r);
}

struct rir *rir_create(struct analyzer *a)
{
    struct rir *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_init(ret, a)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_deinit(struct rir *r)
{
    ast_node_destroy(r->root);
    rf_fixed_memorypool_destroy(r->symbol_table_records_pool);
    rf_fixed_memorypool_destroy(r->types_pool);
    string_table_destroy(r->identifiers_table);
    string_table_destroy(r->string_literals_table);

    rf_objset_clear(r->types_set);
    free(r->types_set);

    rir_types_list_deinit(&r->rir_types_list);
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}


static void rir_finalize_fndecl(struct ast_node *n)
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

static enum traversal_cb_res rir_finalize_do(struct ast_node *n, void *user_arg)
{
    struct rir *rir = user_arg;
    (void)rir;
    switch (n->type) {
    case AST_FUNCTION_DECLARATION:
        rir_finalize_fndecl(n);
        break;
    default:
        break;
    }
    // finally set the state
    n->state = AST_NODE_STATE_RIR_END;
    return TRAVERSAL_CB_OK;
}

static bool do_nothing(struct ast_node *n, void *user_arg) { return true; }

bool rir_finalize_ast(struct rir *r)
{
    // TODO: if we don't have any actual pre_ callback then use ast_post_traverse_tree()
    bool ret = (TRAVERSAL_CB_OK == ast_traverse_tree_nostop_post_cb(
                    r->root,
                    do_nothing,
                    NULL,
                    rir_finalize_do,
                    r
                )
    );
    return ret;
}
