#include "symbol_table_creation.h"

#include <analyzer/analyzer.h>
#include <ast/ast.h>

#include "analyzer_utils.h"

static bool analyzer_populate_symbol_tables_do(struct ast_node *n,
                                               void *user_arg);

struct st_creation_ctx {
    struct analyzer *a;
    struct symbol_table *current_st;
};

static inline void st_creation_ctx_init(struct st_creation_ctx *ctx,
                                        struct analyzer *a)
{
    ctx->a = a;
    ctx->current_st = NULL;
}

bool analyzer_populate_symbol_tables(struct analyzer *a)
{
    struct st_creation_ctx ctx;
    st_creation_ctx_init(&ctx, a);

    return analyzer_pre_traverse_tree(a,
                                      analyzer_populate_symbol_tables_do,
                                      &ctx);
}

static bool analyzer_populate_symbol_tables_do(struct ast_node *n,
                                               void *user_arg)
{
    struct st_creation_ctx *ctx = (struct st_creation_ctx*)user_arg;

    // act depending on the node type
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_ROOT:
        ctx->current_st = &n->root.st;
        break;
    case AST_BLOCK:
        symbol_table_set_parent(&n->block.st, ctx->current_st);
        ctx->current_st = &n->block.st;
        break;
    case AST_TYPE_DECLARATION:
        symbol_table_set_parent(&n->block.st, ctx->current_st);
        ctx->current_st = &n->block.st;
        break;

        // nodes that actually contribute records to symbol tables
    case AST_TYPE_DESCRIPTION:
        if (n->typedesc.left->type != AST_IDENTIFIER) {
            analyzer_err(ctx->a, ast_node_startmark(n->typedesc.left),
                         ast_node_endmark(n->typedesc.left),
                         "Expected an identifier in type description node but "
                         "found a \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(ast_node_str(n->typedesc.left)));
            return false;
        }

        if (!symbol_table_add_node(ctx->current_st,
                                   ast_identifier_str(n->typedesc.left),
                                   n->typedesc.right)) {
            RF_ERROR("Could not add type identifier \""RF_STR_PF_FMT"\" to "
                     "a symbol table",
                     RF_STR_PF_ARG(ast_identifier_str(n->typedesc.left)));
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}
