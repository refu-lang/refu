#include <analyzer/analyzer.h>

#include "analyzer_pass1.h"

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <parser/parser.h>
#include <types/type.h>
#include <analyzer/typecheck.h>
#include <analyzer/string_table.h>

#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048
#define TYPES_POOL_CHUNK_SIZE 2048

i_INLINE_INS void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
                                              struct analyzer *a);

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
    rf_ilist_head_init(&a->composite_types);

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
        // if analyzer has ownership of ast tree
        ast_node_destroy(a->root);
    }
    rf_fixed_memorypool_destroy(a->symbol_table_records_pool);
    rf_fixed_memorypool_destroy(a->types_pool);
    string_table_destroy(a->identifiers_table);
    string_table_destroy(a->string_literals_table);
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

struct type *analyzer_get_or_create_type(struct analyzer *a,
                                         struct ast_node *desc,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(desc, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    rf_ilist_for_each(&a->composite_types, t, lh) {
        if (type_equals_ast_node(t, desc, a, st, genrdecl)) {
            return t;
        }
    }

    // else we have to create a new type
    t = type_create(desc, a, st, genrdecl);
    if (!t) {
        RF_ERROR("Failure to create an composite type");
        return NULL;
    }

    // add it to the list
    rf_ilist_add(&a->composite_types, &t->lh);
    return t;
}

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser)
{
    // acquire the root of the AST from the parser
    a->root = parser_yield_ast_root(parser);

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


i_INLINE_INS void analyzer_set_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error_reset(struct analyzer *a);
i_INLINE_INS struct ast_node *analyzer_yield_ast_root(struct analyzer *analyzer);
