#include <analyzer/analyzer.h>

#include "global_context.h"
#include "analyzer_pass1.h"

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <ast/type.h>
#include <ast/function.h>
#include <ast/ast_utils.h>
#include <parser/parser.h>
#include <types/type.h>
#include <types/type_comparisons.h>
#include <analyzer/typecheck.h>
#include <analyzer/string_table.h>

#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048
#define TYPES_POOL_CHUNK_SIZE 2048

i_INLINE_INS void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
                                              struct analyzer *a);
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

bool analyzer_init(struct analyzer *a, struct info_ctx *info)
{
    a->info = info;
    a->root = NULL;
    a->have_semantic_err = false;

    a->symbol_table_records_pool = rf_fixed_memorypool_create(sizeof(struct symbol_table_record),
                                                              RECORDS_TABLE_POOL_CHUNK_SIZE);
    if (!a->symbol_table_records_pool) {
        RF_ERROR("Failed to initialize a fixed memory pool for symbol records");
        return false;
    }
    a->types_pool = rf_fixed_memorypool_create(sizeof(struct type),
                                               TYPES_POOL_CHUNK_SIZE);
    if (!a->types_pool) {
        RF_ERROR("Failed to initialize a fixed memory pool for types");
        return false;
    }
    RF_MALLOC(a->types_set, sizeof(*a->types_set), return false);
    rf_objset_init(a->types_set, type);

    if (!(a->identifiers_table = string_table_create())) {
        RF_ERROR("Failed to allocate a string table for identifiers");
        return false;
    }
    if (!(a->string_literals_table = string_table_create())) {
        RF_ERROR("Failed to allocate a string table for string literals");
        return false;
    }

    a->warn_on_implicit_conversions = DEFAULT_WARN_ON_IMPLICIT_CONVERSIONS;
    return true;
}

struct analyzer *analyzer_create(struct info_ctx *info)
{
    struct analyzer *a;
    RF_MALLOC(a, sizeof(*a), return NULL);

    if (!analyzer_init(a, info)) {
        free(a);
        return NULL;
    }

    return a;
}


void analyzer_deinit(struct analyzer *a)
{
    if (a->root) {
        ast_node_destroy(a->root);
    }
    if (a->symbol_table_records_pool) {
        rf_fixed_memorypool_destroy(a->symbol_table_records_pool);
    }
    if (a->types_pool) {
        rf_fixed_memorypool_destroy(a->types_pool);
    }
    if (a->identifiers_table) {
        string_table_destroy(a->identifiers_table);
    }
    if (a->string_literals_table) {
        string_table_destroy(a->string_literals_table);
    }

    if (a->types_set) {
        rf_objset_clear(a->types_set);
        free(a->types_set);
    }
}

void analyzer_destroy(struct analyzer *a)
{
    analyzer_deinit(a);
    free(a);
}

struct inpfile *analyzer_get_file(struct analyzer *a)
{
    return a->info->file;
}

// TODO: Maybe delete this function and simply use rf_objset_add() ?
bool analyzer_types_set_add(struct analyzer *a, struct type *new_type)
{
    return rf_objset_add(a->types_set, type, new_type);
}

struct type *analyzer_get_or_create_type(struct analyzer *a,
                                         const struct ast_node *desc,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl)
{
    struct type *t;
    struct rf_objset_iter it;
    AST_NODE_ASSERT_TYPE(desc, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    rf_objset_foreach(a->types_set, &it, t) {
        if (type_equals_ast_node(t, desc, a, st, genrdecl, TYPECMP_GENERIC)) {
            return t;
        }
    }

    // else we have to create a new type
    t = type_create_from_node(desc, a, st, genrdecl);
    if (!t) {
        RF_ERROR("Failure to create a composite type");
        return NULL;
    }

    // add it to the list
    analyzer_types_set_add(a, t);
    if (desc->type == AST_TYPE_OPERATOR && ast_typeop_op(desc) == TYPEOP_SUM) {
        // if it's a sum type also add the left and the right operand type
        analyzer_types_set_add(a, t->operator.left);
        analyzer_types_set_add(a, t->operator.right);
    }
    return t;
}

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser,
                           bool with_global_context)
{
    // acquire the root of the AST from the parser
    a->root = parser_yield_ast_root(parser);

    // initialize root symbol table here instead od analyzer_first_pass
    // since we need it at least for now if we want to introduce a global context
    if (!ast_root_symbol_table_init(a->root, a)) {
        RF_ERROR("Could not initialize symbol table for root node");
        return false;
    }
    if (with_global_context) {
        if (!analyzer_load_globals(a)) {
            RF_ERROR("Failure at loading the global context for the analyzer");
            return false;
        }
    }
    // create symbol tables and change ast nodes ownership
    if (!analyzer_first_pass(a)) {
        RF_ERROR("Failure at analyzer's first pass");
        return false;
    }

    if (!analyzer_typecheck(a, a->root)) {
        RF_ERROR("Failure at analyzer's typechecking");
        return false;
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
    struct analyzer *a = user_arg;
    (void)a;
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

bool analyzer_finalize(struct analyzer *a)
{
    rir_types_list_init(&a->rir_types_list);
    // TODO: if we don't have any actual pre_ callback then use ast_post_traverse_tree()
    bool ret = (TRAVERSAL_CB_OK == ast_traverse_tree_nostop_post_cb(
                    a->root,
                    do_nothing,
                    NULL,
                    analyzer_finalize_do,
                    a
                )
    );
    return ret;    
}


i_INLINE_INS void analyzer_set_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error_reset(struct analyzer *a);
i_INLINE_INS struct ast_node *analyzer_yield_ast_root(struct analyzer *analyzer);
