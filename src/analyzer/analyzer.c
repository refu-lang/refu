#include <analyzer/analyzer.h>

#include "symbol_table_creation.h"

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <parser/parser.h>

#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048

bool analyzer_init(struct analyzer *a, struct info_ctx *info)
{
    bool rc;
    a->info = info;
    a->root = NULL;
    a->have_semantic_err = false;

    rc = rf_fixed_memorypool_init(&a->symbol_table_records_pool,
                                  sizeof(struct symbol_table_record),
                                  RECORDS_TABLE_POOL_CHUNK_SIZE);
    return rc == true;
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

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser)
{
    // acquire the root of the AST from the parser
    a->root = parser_yield_ast_root(parser);
    
    // create symbol tables and change ast nodes ownership
    analyzer_create_symbol_tables(a);
    return true;
}


i_INLINE_INS void analyzer_set_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error_reset(struct analyzer *a);
