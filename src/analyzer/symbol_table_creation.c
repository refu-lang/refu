#include "symbol_table_creation.h"

#include <analyzer/analyzer.h>

#include <ast/ast.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/function.h>

#include "analyzer_utils.h"

static bool analyzer_create_symbol_tables_do(struct ast_node *n,
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

static bool analyzer_create_symbol_tables_do(struct ast_node *n,
                                             void *user_arg)
{
    struct symbol_table *st;
    struct st_creation_ctx *ctx = (struct st_creation_ctx*)user_arg;

    // since this is the very first phase of the analyzer and should happen
    // only once, change node ownership here
    n->owner = AST_OWNEDBY_ANALYZER;

    // act depending on the node type
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_ROOT:
        if (!ast_root_symbol_table_init(n, ctx->a)) {
           RF_ERROR("Could not initialize symbol table for root node");
           return false;
        }
        RF_ASSERT(ctx->current_st == NULL, "Visiting root node more than once");
        ctx->current_st = ast_root_symbol_table_get(n);
        break;
    case AST_BLOCK:
        if (!ast_block_symbol_table_init(n, ctx->a)) {
            RF_ERROR("Could not initialize symbol table for block node");
            return false;
        }
        st = ast_block_symbol_table_get(n);
        symbol_table_set_parent(st, ctx->current_st);
        ctx->current_st = st;
        break;
    case AST_TYPE_DECLARATION:
        if (!ast_typedecl_symbol_table_init(n, ctx->a)) {
            RF_ERROR("Could not initialize symbol table for type declaration node");
            return false;
        }
        st = ast_typedecl_symbol_table_get(n);
        symbol_table_set_parent(st, ctx->current_st);
        ctx->current_st = st;
        break;
    case AST_FUNCTION_DECLARATION:
        if (!ast_fndecl_symbol_table_init(n, ctx->a)) {
            RF_ERROR("Could not initialize symbol table for function declaration node");
            return false;
        }
        st = ast_fndecl_symbol_table_get(n);
        symbol_table_set_parent(st, ctx->current_st);
        ctx->current_st = st;
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

bool analyzer_create_symbol_tables(struct analyzer *a)
{
    struct st_creation_ctx ctx;
    st_creation_ctx_init(&ctx, a);

    return analyzer_pre_traverse_tree(a,
                                      analyzer_create_symbol_tables_do,
                                      &ctx);
}
