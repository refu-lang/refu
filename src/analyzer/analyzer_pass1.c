#include "analyzer_pass1.h"

#include <analyzer/analyzer.h>

#include <ast/ast.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/function.h>
#include <ast/string_literal.h>
#include <ast/ast_utils.h>

#include <types/type_function.h>

static bool analyzer_first_pass_do(struct ast_node *n,
                                   void *user_arg);

static bool analyzer_create_symbol_table_typedecl(struct analyzer_traversal_ctx *ctx,
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

    // also initialize the symbol table of the type declaration
    if (!symbol_table_init(&n->typedecl.st, ctx->a)) {
        RF_ERROR("Failed to initialize type declaration symbol table");
        return false;
    }

    return true;
}

static bool analyzer_populate_symbol_table_typeleaf(struct analyzer_traversal_ctx *ctx,
                                                    struct ast_node *n)
{
    struct ast_node *search_node;
    bool symbol_found_at_first_st;
    const struct RFstring *id_name;
    struct ast_node *left;
    RF_ASSERT(n->type == AST_TYPE_DESCRIPTION &&
              (left=ast_typedesc_left(n))->type == AST_IDENTIFIER,
              "Called function for non typeleaf");
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

    if (!symbol_table_add_node(ctx->current_st, ctx->a, id_name, n)) {
        RF_ERROR("Could not add a type leaf's  node to a symbol table");
        return false;
    }

    return true;
}

static bool analyzer_populate_symbol_table_vardecl(struct analyzer_traversal_ctx *ctx,
                                                   struct ast_node *n)
{
    struct ast_node *left;
    struct ast_node *desc;
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);

    desc = ast_vardecl_desc_get(n);
    left = ast_types_left(desc);
    if (desc->type != AST_TYPE_DESCRIPTION || left->type != AST_IDENTIFIER) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier in the left side of a variable's  "
                     "type description node but "
                     "found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
            return false;
    }
    return analyzer_populate_symbol_table_typeleaf(ctx, desc);
}

static bool analyzer_populate_symbol_table_typedesc(struct analyzer_traversal_ctx *ctx,
                                                    struct ast_node *n)
{
    struct ast_node *left;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DESCRIPTION || AST_TYPE_OPERATOR);
    left = ast_types_left(n);
    if (n->type == AST_TYPE_DESCRIPTION) {
        if (left->type == AST_IDENTIFIER) {
            return analyzer_populate_symbol_table_typeleaf(ctx, n);
        } else {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier in the left side of a type "
                     "description node but "
                     "found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
            return false;
        }
    }

    // should be a type operator
    return analyzer_populate_symbol_table_typedesc(ctx,left) &&
           analyzer_populate_symbol_table_typedesc(ctx, ast_typeop_right(n));
}

static bool analyzer_create_symbol_table_fndecl(struct analyzer_traversal_ctx *ctx,
                                                struct ast_node *n)
{
    struct symbol_table_record *rec;
    const struct RFstring *fn_name;
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);

    // initialize this function's symbol table
    if (!ast_fndecl_symbol_table_init(n, ctx->a)) {
        RF_ERROR("Could not initialize symbol table for function declaration node");
        return false;
    }

    fn_name = ast_fndecl_name_str(n);
    rec = symbol_table_lookup_record(ctx->current_st, fn_name, NULL);

    if (rec && type_is_function(rec->data)) {
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

    // function's arguments are added to the symbol table by type creation
    return true;
}

static bool analyzer_first_pass_do(struct ast_node *n,
                                   void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = user_arg;

    // act depending on the node type
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_ROOT:
        RF_ASSERT(ctx->current_st == NULL, "Visiting root node more than once");
        // symbol table already initialized in analyzer_analyze_file
        ctx->current_st = ast_root_symbol_table_get(n);
        break;
    case AST_BLOCK:
        if (!ast_block_symbol_table_init(n, ctx->a)) {
            RF_ERROR("Could not initialize symbol table for block node");
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_block_symbol_table_get(n));
        break;
    case AST_FUNCTION_DECLARATION:
        if (!analyzer_create_symbol_table_fndecl(ctx, n)) {
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_fndecl_symbol_table_get(n));
        symbol_table_set_fndecl(ctx->current_st, n);
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        // function implementation symbol table should point to its decl table
        ast_fnimpl_symbol_table_set(n, ast_fndecl_symbol_table_get(n->fnimpl.decl));
        break;
    case AST_TYPE_DECLARATION:
        if (!analyzer_create_symbol_table_typedecl(ctx, n)) {
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_typedecl_symbol_table_get(n));
        // also populate the type declaration's symbol table
        if (!analyzer_populate_symbol_table_typedesc(ctx, ast_typedecl_typedesc_get(n))) {
            return false;
        }
        break;

    // nodes that only contribute records to symbol tables
    case AST_VARIABLE_DECLARATION:
        if (!analyzer_populate_symbol_table_vardecl(ctx, n)) {
            return false;
        }
        break;
    case AST_IDENTIFIER:
        // create identifier hash and dissasociate from the file
        RF_ASSERT(n->owner == AST_OWNEDBY_PARSER,
                  "Attempting to create identifier hash for node in a wrong "
                  "state of processing");
        if (!ast_identifier_hash_create(n, ctx->a)) {
            return false;
        }
        break;
    case AST_STRING_LITERAL:
        // create literal hash and dissasociate from the file
        RF_ASSERT(n->owner == AST_OWNEDBY_PARSER,
                  "Attempting to create literal hash for node in a wrong state "
                  "of processing");
        if (!ast_string_literal_hash_create(n, ctx->a)) {
            return false;
        }
        break;
    default:
        // do nothing
        break;
    }

    // since this is the very first pass of the analyzer and should happen
    // only once, change node ownership here
    n->owner = AST_OWNEDBY_ANALYZER_PASS1;
    return true;
}

bool analyzer_handle_symbol_table_ascending(struct ast_node *n,
                                            struct analyzer_traversal_ctx *ctx)
{
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_BLOCK:
        ctx->current_st = ast_block_symbol_table_get(n)->parent;
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        ctx->current_st = ast_fnimpl_symbol_table_get(n)->parent;
        break;
    case AST_FUNCTION_DECLARATION:
        // When the function declaration is inside a function implementation it
        // should not go upwards here since it would mess up current symbol table
        // for the function's block
        if (ast_fndecl_position_get(n) != FNDECL_PARTOF_IMPL) {
            ctx->current_st = ast_fndecl_symbol_table_get(n)->parent;
        }
        break;
    case AST_TYPE_DECLARATION:
        ctx->current_st = ast_typedecl_symbol_table_get(n)->parent;
        break;
    default:
        // do nothing
        break;
    }

    RF_ASSERT(ctx->current_st, "Symbol table movement to parent lead to "
              "a NULL table!");
    return true;
}

bool analyzer_handle_symbol_table_descending(struct ast_node *n,
                                             struct analyzer_traversal_ctx *ctx)
{
    darray_append(ctx->parent_nodes, n);
    switch(n->type) {
    case AST_ROOT:
        ctx->current_st = ast_root_symbol_table_get(n);
        break;
    case AST_BLOCK:
        ctx->current_st = ast_block_symbol_table_get(n);
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        ctx->current_st = ast_fnimpl_symbol_table_get(n);
        break;
    case AST_FUNCTION_DECLARATION:
        ctx->current_st = ast_fndecl_symbol_table_get(n);
        break;
    case AST_TYPE_DECLARATION:
        ctx->current_st = ast_typedecl_symbol_table_get(n);
        break;
    default:
        break;
    }
    return true;
}

bool analyzer_first_pass(struct analyzer *a)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, a);

    bool ret = ast_traverse_tree(
        a->root,
        analyzer_first_pass_do,
        &ctx,
        (ast_node_cb)analyzer_handle_symbol_table_ascending,
        &ctx);

    analyzer_traversal_ctx_deinit(&ctx);
    return ret;
}
