#include <analyzer/analyzer_pass1.h>

#include <analyzer/analyzer.h>

#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/block.h>
#include <ast/type.h>
#include <ast/vardecl.h>
#include <ast/function.h>
#include <ast/matchexpr.h>
#include <ast/forexpr.h>
#include <ast/string_literal.h>
#include <ast/module.h>
#include <ast/typeclass.h>
#include <ast/ast_utils.h>

#include <analyzer/typecheck_forexpr.h>
#include <types/type_function.h>

static inline void analyzer_traversal_ctx_prev_parent(struct analyzer_traversal_ctx *ctx)
{
    RF_ASSERT(darray_size(ctx->parent_nodes) != 0, "Tried to go beyond the root");
    (void)darray_pop(ctx->parent_nodes);
}

static inline void analyzer_traversal_ctx_push_parent(struct analyzer_traversal_ctx *ctx,
                                                      struct ast_node *n)
{
    darray_append(ctx->parent_nodes, n);
}

static bool analyzer_first_pass_do(struct ast_node *n, void *user_arg);

static bool analyzer_populate_symbol_table_typedecl(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    const struct ast_node *search_node;
    bool symbol_found_at_first_st;
    const struct RFstring *type_name;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);

    type_name = ast_typedecl_name_str(n);
    search_node = symbol_table_lookup_node(
        ctx->current_st,
        type_name,
        &symbol_found_at_first_st
    );

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(ctx->m, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Type \""RFS_PF"\" was already declared in scope "
                     "at "INPLOCATION_FMT,
                     RFS_PA(type_name),
                     INPLOCATION_ARG(module_get_file(ctx->m),
                                     ast_node_location(search_node)));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->m, type_name, n)) {
        if (!module_have_errors(ctx->m)) {
            RF_ERROR("Could not add a typedecl node to a symbol table");
        }
        return false;
    }

    return true;
}


static bool analyzer_populate_symbol_module(struct analyzer_traversal_ctx *ctx,
                                            struct ast_node *n)
{
    const struct ast_node *search_node;
    bool symbol_found_at_first_st;
    const struct RFstring *name = ast_module_name(n);

    search_node = symbol_table_lookup_node(
        ctx->current_st,
        name,
        &symbol_found_at_first_st
    );

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(ctx->m, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Identifier \""RFS_PF"\" was already used in scope "
                     "at "INPLOCATION_FMT,
                     RFS_PA(name),
                     INPLOCATION_ARG(module_get_file(ctx->m),
                                     ast_node_location(search_node)));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->m, name, n)) {
        RF_ERROR("Could not add a module to a symbol table");
        return false;
    }

    return true;
}

static bool analyzer_populate_symbol_table_typeleaf(struct analyzer_traversal_ctx *ctx,
                                                    struct ast_node *n)
{
    const struct ast_node *search_node;
    bool symbol_found_at_first_st;
    const struct RFstring *id_name;
    struct ast_node *left;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_LEAF);

    left = ast_typeleaf_left(n);
    id_name = ast_identifier_str(left);
    search_node = symbol_table_lookup_node(ctx->current_st,
                                           id_name,
                                           &symbol_found_at_first_st);

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(ctx->m, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Identifier \""RFS_PF"\" was already used in scope "
                     "at "INPLOCATION_FMT,
                     RFS_PA(id_name),
                     INPLOCATION_ARG(module_get_file(ctx->m),
                                     ast_node_location(search_node)));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->m, id_name, n)) {
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
    left = ast_typeleaf_left(desc);
    if (left->type != AST_IDENTIFIER) {
        analyzer_err(ctx->m, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier in the left side of a variable's  "
                     "type description node but found a \""RFS_PF"\"",
                     RFS_PA(ast_node_str(left)));
            return false;
    }
    return analyzer_populate_symbol_table_typeleaf(ctx, desc);
}

// populate for generic non-top level type element
static bool analyzer_populate_symbol_table_typeelement(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    switch (n->type) {
    case AST_TYPE_LEAF:
        return analyzer_populate_symbol_table_typeleaf(ctx, n);
    case AST_TYPE_OPERATOR:
        return analyzer_populate_symbol_table_typeelement(ctx, ast_typeop_left(n)) &&
            analyzer_populate_symbol_table_typeelement(ctx, ast_typeop_right(n));
    case AST_TYPE_DESCRIPTION:
        // don't go further into a recursive top level typedesc. Should be handled by the
        // main loop
    case AST_XIDENTIFIER: //fallthrough
        // don't do anything for right part of leaves that are simply identifiers
        return true;
    default:
        RF_CRITICAL_FAIL("Case should never happen");
        return false;
    }
}
// populate for top level type description
static inline bool analyzer_populate_symbol_table_typedesc(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    return analyzer_populate_symbol_table_typeelement(ctx, ast_typedesc_desc_get(n));
}

static bool analyzer_symbol_table_add_fndecl(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    struct symbol_table_record *rec;
    const struct RFstring *fn_name;
    fn_name = ast_fndecl_name_str(n);
    rec = symbol_table_lookup_record(ctx->current_st, fn_name, NULL);

    if (rec && type_is_function(rec->data)) {
        analyzer_err(ctx->m, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Function \""RFS_PF"\" was already declared "
                     "at "INPLOCATION_FMT,
                     RFS_PA(fn_name),
                     INPLOCATION_ARG(
                         module_get_file(ctx->m),
                         ast_node_location(symbol_table_record_node(rec))));
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->m, fn_name, n)) {
        RF_ERROR("Could not add a function node to a symbol table");
        return false;
    }

    // function's arguments are added to the symbol table by type creation
    return true;
}

static bool analyzer_init_and_process_fndecl_symbol_table(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    if (!ast_fndecl_symbol_table_init(n, ctx->m)) {
        RF_ERROR("Could not initialize symbol table for function declaration node");
        return false;
    }
    // add function to the symbol table
    return analyzer_symbol_table_add_fndecl(ctx, n);
}

static bool analyzer_init_and_process_typeclass_symbol_table(
    struct analyzer_traversal_ctx *ctx,
    struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPECLASS_DECLARATION);
    if (!symbol_table_init(&n->typeclass.st, ctx->m)) {
        RF_ERROR("Could not initialize symbol table for typeclass declaration node");
        return false;
    }
    // also add any generics to the typeclass symbol table
    struct ast_node *generics = ast_typeclass_generics(n);
    if (generics) {
        symbol_table_add_genrdecl(&n->typeclass.st, ctx->m, generics);
    }
    // add typeclass to the parent symbol table
    struct symbol_table_record *rec;
    const struct RFstring *typeclass_name = ast_typeclass_name_str(n);
    rec = symbol_table_lookup_record(ctx->current_st, typeclass_name, NULL);

    if (rec && rec->node && rec->node->type == AST_TYPECLASS_DECLARATION) {
        analyzer_err(
            ctx->m, ast_node_startmark(n),
            ast_node_endmark(n),
            "Typeclass \""RFS_PF"\" was already declared "
            "at "INPLOCATION_FMT,
            RFS_PA(typeclass_name),
            INPLOCATION_ARG(
                module_get_file(ctx->m),
                ast_node_location(symbol_table_record_node(rec)))
        );
        return false;
    }

    if (!symbol_table_add_node(ctx->current_st, ctx->m, typeclass_name, n)) {
        RF_ERROR("Could not add a typeclass node to a symbol table");
        return false;
    }

    return true;
}

static bool analyzer_first_pass_do(struct ast_node *n, void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = user_arg;
    analyzer_traversal_ctx_push_parent(ctx, n);
    // act depending on the node type
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_BLOCK:
        if (!ast_block_symbol_table_init(n, ctx->m)) {
            RF_ERROR("Could not initialize symbol table for block node");
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_block_symbol_table_get(n));
        break;
    case AST_MODULE:
        // input the module's name to the symbol table
        if (!analyzer_populate_symbol_module(ctx, n)) {
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_module_symbol_table_get(n));
        break;
    case AST_FOR_EXPRESSION:
        if (!ast_forexpr_symbol_table_init(n, ctx->m)) {
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_forexpr_symbol_table_get(n));
        break;
    case AST_FUNCTION_DECLARATION:
        if (!analyzer_init_and_process_fndecl_symbol_table(ctx, n)) {
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_fndecl_symbol_table_get(n));
        symbol_table_set_fndecl(ctx->current_st, n);
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        // function implementation symbol table should point to its decl table
        ast_fnimpl_symbol_table_set(n, ast_fndecl_symbol_table_get(n->fnimpl.decl));
        break;
    case AST_MATCH_CASE:
        // match case symbol table should point to its pattern typedesc table
        ast_matchcase_symbol_table_set(n, ast_typedesc_symbol_table_get(n->matchcase.pattern));
        break;
    case AST_TYPE_DESCRIPTION:
        // initialize the type description's symbol table
        if (!symbol_table_init(&n->typedesc.st, ctx->m)) {
            RF_ERROR("Could not initialize symbol table for top level type description node");
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_typedesc_symbol_table_get(n));
        // also populate the type description's symbol table
        if (!analyzer_populate_symbol_table_typedesc(ctx, n)) {
            RF_ERROR("Could not populate symbol table for top level type description node");
            return false;
        }
        break;
    case AST_TYPE_DECLARATION:
        if (!analyzer_populate_symbol_table_typedecl(ctx, n)) {
            return false;
        }
        break;
    case AST_TYPECLASS_DECLARATION:
        // initialize the type class's symbol table
        if (!analyzer_init_and_process_typeclass_symbol_table(ctx, n)) {
            RF_ERROR("Could not initialize symbol table for top level type description node");
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_typeclass_symbol_table_get(n));
        symbol_table_set_fndecl(ctx->current_st, n);
        break;
    case AST_TYPECLASS_INSTANCE:
        // initialize the type class's symbol table
        if (!symbol_table_init(&n->typeinstance.st, ctx->m)) {
            RF_ERROR("Could not initialize symbol table for top level type description node");
            return false;
        }
        symbol_table_swap_current(&ctx->current_st, ast_typeinstance_symbol_table_get(n));
        symbol_table_set_fndecl(ctx->current_st, n);
        break;

    // nodes that only contribute records to symbol tables
    case AST_VARIABLE_DECLARATION:
        if (!analyzer_populate_symbol_table_vardecl(ctx, n)) {
            RF_ERROR("Could not populate symbol table for variable declaration");
            return false;
        }
        break;
    case AST_IDENTIFIER:
        // create identifier hash and dissasociate from the file
        RF_ASSERT(n->state == AST_NODE_STATE_AFTER_PARSING,
                  "Attempting to create identifier hash for node in a wrong "
                  "state of processing");
        if (!ast_identifier_hash_create(n, ctx->m)) {
            return false;
        }
        break;
    case AST_STRING_LITERAL:
        // create literal hash and dissasociate from the file
        RF_ASSERT(n->state == AST_NODE_STATE_AFTER_PARSING,
                  "Attempting to create literal hash for node in a wrong state "
                  "of processing");
        if (!ast_string_literal_hash_create(n, ctx->m)) {
            return false;
        }
        break;
    default:
        // do nothing
        break;
    }

    // since this is the very first pass of the analyzer and should happen
    // only once, change node ownership here
    n->state = AST_NODE_STATE_ANALYZER_PASS1;
    return true;
}

bool analyzer_handle_symbol_table_ascending(
    struct ast_node *n,
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
    case AST_MATCH_CASE:
        ctx->current_st = ast_matchcase_symbol_table_get(n)->parent;
        break;
    case AST_FUNCTION_DECLARATION:
        // When the function declaration is inside a function implementation it
        // should not go upwards here since it would mess up current symbol table
        // for the function's block
        if (ast_fndecl_position_get(n) != FNDECL_PARTOF_IMPL) {
            ctx->current_st = ast_fndecl_symbol_table_get(n)->parent;
        }
        break;
    case AST_MODULE:
            ctx->current_st = ast_module_symbol_table_get(n)->parent;
        break;
    case AST_TYPE_DESCRIPTION:
    {
        struct ast_node *parent = analyzer_traversal_ctx_get_nth_parent(0, ctx);
        // If this is the type description of a match case pattern's then
        // don't go to parent symbol table here, since when going downwards
        // again for the match case expression we would be pointing to the
        // enclosing block's symbol table
        if (!parent || parent->type != AST_MATCH_CASE) {
            ctx->current_st = ast_typedesc_symbol_table_get(n)->parent;
        }
    }
    break;
    case AST_FOR_EXPRESSION:
    case AST_TYPECLASS_DECLARATION:
    case AST_TYPECLASS_INSTANCE:
        ctx->current_st = ast_node_symbol_table_get(n)->parent;
        break;
    default:
        // do nothing
        break;
    }

    // go back to previous parent
    analyzer_traversal_ctx_prev_parent(ctx);
    return true;
}

bool analyzer_handle_traversal_descending(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    analyzer_traversal_ctx_push_parent(ctx, n);
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
    case AST_MATCH_CASE:
        ctx->current_st = ast_matchcase_symbol_table_get(n);
        break;
    case AST_FUNCTION_DECLARATION:
        ctx->current_st = ast_fndecl_symbol_table_get(n);
        break;
    case AST_MODULE:
        ctx->current_st = ast_module_symbol_table_get(n);
        break;
    case AST_TYPE_DESCRIPTION:
        ctx->current_st = ast_typedesc_symbol_table_get(n);
        break;
    case AST_TYPECLASS_DECLARATION:
        ctx->current_st = ast_typeclass_symbol_table_get(n);
        break;
    case AST_TYPECLASS_INSTANCE:
        ctx->current_st = ast_typeinstance_symbol_table_get(n);
        break;
    case AST_MATCH_EXPRESSION:
        return pattern_matching_ctx_init(&ctx->matching_ctx, ctx->current_st, n);
    case AST_FOR_EXPRESSION:
        return typecheck_forexpr_descending(n, ctx);
    default:
        break;
    }
    return true;
}

bool analyzer_first_pass(struct module *m)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, m);
    // set the starting symbol_table
    ctx.current_st = &m->front->root->root.st;

    bool ret = ast_traverse_tree(
        m->node,
        analyzer_first_pass_do,
        &ctx,
        (ast_node_cb)analyzer_handle_symbol_table_ascending,
        &ctx);

    analyzer_traversal_ctx_deinit(&ctx);
    return ret;
}
