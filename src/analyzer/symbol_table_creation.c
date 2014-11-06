#include "symbol_table_creation.h"

#include <analyzer/analyzer.h>

#include <ast/ast.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
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


static bool analyzer_create_symbol_table_typedecl(struct st_creation_ctx *ctx,
                                                  struct ast_node *n)
{
    struct ast_node *search_node;
    bool symbol_found_at_first_st;
    const struct RFstring *type_name;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);

    type_name = ast_typedecl_name_str(n);
    search_node = symbol_table_lookup_node(ctx->current_st,
                                           type_name,
                                           &symbol_found_at_first_st);

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Type \""RF_STR_PF_FMT"\" was already declared in scope "
                     "at "INPLOCATION_FMT,
                     RF_STR_PF_ARG(type_name),
                     INPLOCATION_ARG(analyzer_get_file(ctx->a),
                                     ast_node_location(search_node)));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->a, type_name, n)) {
        RF_ERROR("Could not add a typedecl node to a symbol table");
        return false;
    }

    return true;
}

static bool analyzer_create_symbol_table_vardecl(struct st_creation_ctx *ctx,
                                                 struct ast_node *n)
{
    struct ast_node *search_node;
    bool symbol_found_at_first_st;
    struct ast_node *left;
    struct ast_node *desc;
    const struct RFstring *id_name;
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);

    desc = ast_vardecl_desc_get(n);
    left = ast_typedesc_left(desc);

    if (left->type != AST_IDENTIFIER) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier in the left side of a type "
                     "description node but "
                     "found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
        return false;
    }

    id_name = ast_identifier_str(left);
    search_node = symbol_table_lookup_node(ctx->current_st,
                                           id_name,
                                           &symbol_found_at_first_st);

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Identifier \""RF_STR_PF_FMT"\" was already used in scope "
                     "at "INPLOCATION_FMT,
                     RF_STR_PF_ARG(id_name),
                     INPLOCATION_ARG(analyzer_get_file(ctx->a),
                                     ast_node_location(search_node)));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->a,
                               id_name, n)) {
        RF_ERROR("Could not add a type description's left node "
                 "to a symbol table");
        return false;
    }

    return true;
}

static bool analyzer_create_symbol_table_fndecl(struct st_creation_ctx *ctx,
                                                struct ast_node *n)
{
    struct symbol_table *st;
    struct symbol_table_record *rec;
    bool symbol_found_at_first_st;
    const struct RFstring *fn_name;
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);

    // initialize this function's symbol table
    if (!ast_fndecl_symbol_table_init(n, ctx->a)) {
        RF_ERROR("Could not initialize symbol table for function declaration node");
        return false;
    }

    fn_name = ast_fndecl_name_str(n);
    rec = symbol_table_lookup_record(ctx->current_st,
                                     fn_name,
                                     &symbol_found_at_first_st);

    if (rec && symbol_table_record_category(rec) == TYPE_CATEGORY_FUNCTION) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Function \""RF_STR_PF_FMT"\" was already declared "
                     "at "INPLOCATION_FMT,
                     RF_STR_PF_ARG(fn_name),
                     INPLOCATION_ARG(
                         analyzer_get_file(ctx->a),
                         ast_node_location(symbol_table_record_node(rec))));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->a, fn_name, n)) {
        RF_ERROR("Could not add a function node to a symbol table");
        return false;
    }

    // set the current symbol table to the function's one
    st = ast_fndecl_symbol_table_get(n);
    symbol_table_set_parent(st, ctx->current_st);
    ctx->current_st = st;

    return true;
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
    case AST_FUNCTION_DECLARATION:
        if (!analyzer_create_symbol_table_fndecl(ctx, n)) {
            return false;
        }
        break;

        // nodes that only contribute records to symbol tables
    case AST_TYPE_DECLARATION:
        if (!analyzer_create_symbol_table_typedecl(ctx, n)) {
            return false;
        }
        break;
    case AST_VARIABLE_DECLARATION:
        if (!analyzer_create_symbol_table_vardecl(ctx, n)) {
            return false;
        }
        break;
    default:
        // do nothing
        break;
    }

    return true;
}

static bool analyzer_switch_current_symbol_table(struct ast_node *n,
                                                 void *user_arg)
{
    struct st_creation_ctx *ctx = (struct st_creation_ctx*)user_arg;
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_BLOCK:
        ctx->current_st = ast_block_symbol_table_get(n)->parent;
        break;
    case AST_FUNCTION_DECLARATION:
        ctx->current_st = ast_fndecl_symbol_table_get(n)->parent;
        break;
    default:
        // do nothing
        break;
    }

    RF_ASSERT(ctx->current_st, "Symbol table movement to parent lead to "
              "a NULL table!");
    return true;
}

bool analyzer_create_symbol_tables(struct analyzer *a)
{
    struct st_creation_ctx ctx;
    st_creation_ctx_init(&ctx, a);

    return analyzer_traverse_tree(a,
                                  analyzer_create_symbol_tables_do,
                                  &ctx,
                                  analyzer_switch_current_symbol_table,
                                  &ctx);
}
