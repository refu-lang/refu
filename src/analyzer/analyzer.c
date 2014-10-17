#include <analyzer/analyzer.h>

#include "symbol_table_creation.h"

#include <Utils/memory.h>

#include <ast/ast.h>
#include <parser/parser.h>

bool analyzer_init(struct analyzer *a, struct info_ctx *info)
{
    a->info = info;
    a->root = NULL;
    a->have_semantic_err = false;
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
    //TODO: if we don't deinit anything then, simply get rid of this function
    return;
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

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser)
{
    // acquire the root of the AST from the parser
    a->root = parser_yield_ast_root(parser);
    
    analyzer_populate_symbol_tables(a);
    return true;
}


i_INLINE_INS void analyzer_set_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error(struct analyzer *a);
i_INLINE_INS bool analyzer_has_semantic_error_reset(struct analyzer *a);
