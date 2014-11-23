#include <analyzer/analyzer.h>

#include "analyzer_pass1.h"

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <parser/parser.h>
#include <analyzer/types.h>
#include <analyzer/typecheck.h>

#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048
#define TYPES_POOL_CHUNK_SIZE 2048

i_INLINE_INS void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
                                              struct analyzer *a);

bool analyzer_init(struct analyzer *a, struct info_ctx *info)
{
    bool rc;
    a->info = info;
    a->root = NULL;
    a->have_semantic_err = false;

    rc = rf_fixed_memorypool_init(&a->symbol_table_records_pool,
                                  sizeof(struct symbol_table_record),
                                  RECORDS_TABLE_POOL_CHUNK_SIZE);
    if (!rc) {
        return false;
    }
    rc = rf_fixed_memorypool_init(&a->types_pool,
                                  sizeof(struct type),
                                  TYPES_POOL_CHUNK_SIZE);
    if (!rc) {
        return false;
    }

    rf_ilist_head_init(&a->anonymous_types);
    rf_ilist_head_init(&a->types);

    if (!string_table_init(&a->identifiers_table)) {
        return false;
    }
    if (!string_table_init(&a->string_literals_table)) {
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
    rf_fixed_memorypool_deinit(&a->symbol_table_records_pool);
    rf_fixed_memorypool_deinit(&a->types_pool);
    string_table_deinit(&a->identifiers_table);
    string_table_deinit(&a->string_literals_table);
}

void analyzer_destroy(struct analyzer *a)
{
    if (a->root) {
        // if analyzer has ownership of ast tree
        ast_node_destroy(a->root);
    }

    analyzer_deinit(a);
    free(a);
}

struct inpfile *analyzer_get_file(struct analyzer *a)
{
    return a->info->file;
}

struct type *analyzer_get_or_create_anonymous_type(struct analyzer *a,
                                                   struct ast_node *desc,
                                                   struct symbol_table *st,
                                                   struct ast_node *genrdecl)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(desc, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    rf_ilist_for_each(&a->anonymous_types, t, lh) {
        if (type_equals_typedesc(t, desc, a, st, genrdecl)) {
            return t;
        }
    }

    // else we have to create a new anonymous type
    t = type_anonymous_create(desc, a, st, genrdecl);
    if (!t) {
        RF_ERROR("Failure to create an anonymous type");
        return NULL;
    }

    // add it to the list
    rf_ilist_add(&a->anonymous_types, &t->lh);
    return t;
}

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser)
{
    // acquire the root of the AST from the parser
    a->root = parser_yield_ast_root(parser);

    // create symbol tables and change ast nodes ownership
    analyzer_first_pass(a);

    //TODO: type check
    analyzer_typecheck(a);

    return true;
}


i_INLINE_INS void analyzer_set_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error_reset(struct analyzer *a);
i_INLINE_INS struct ast_node *analyzer_yield_ast_root(struct analyzer *analyzer);
