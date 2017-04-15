#ifndef LFR_ANALYZER_H
#define LFR_ANALYZER_H

#include <stdbool.h>

#include <rfbase/datastructs/darray.h>
#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>

#include <front_ctx.h>
#include <analyzer/typecheck_matchexpr.h>
#include <analyzer/typecheck_typeclass.h>
#include <analyzer/type_set.h>
#include <ast/ast_utils.h>

struct parser;
struct inpfile;
struct type;
struct symbol_table;
struct analyzer;
struct module;

//! The size in bytes of the types memory pool
#define TYPES_POOL_CHUNK_SIZE 2048
//! The size in bytes of the symbol table records memory pool
#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048

struct analyzer_traversal_ctx {
    struct module *m;
    //! Remembers the current symbol table during ast traversal
    struct symbol_table *current_st;
    //! Remembers the type of the previous node during the typechecking
    //! Used for assigning types to blocks
    const struct type *last_node_type;
    //! A queue of nodes to remember the current parent of a node during traversal
    struct arr_ast_nodes parent_nodes;
    //! Pattern matching related data
    struct pattern_matching_ctx matching_ctx;
    //! Typeclass instantiation related data
    struct typeclass_ctx typeclass_ctx;
};

i_INLINE_DECL void analyzer_traversal_ctx_init(
    struct analyzer_traversal_ctx *ctx,
    struct module *m)
{
    ctx->m = m;
    ctx->current_st = NULL;
    ctx->last_node_type = NULL;
    darray_init(ctx->parent_nodes);
}

i_INLINE_DECL void analyzer_traversal_ctx_deinit(struct analyzer_traversal_ctx *ctx)
{
    darray_free(ctx->parent_nodes);
}

/**
 * Gets the @a num parent going upwards from the node. Indexing starts from 0.
 * e.g.: To get the direct parent give num == 0
 *
 */
i_INLINE_DECL struct ast_node *analyzer_traversal_ctx_get_nth_parent(
    unsigned int num,
    struct analyzer_traversal_ctx *ctx)
{
    if ((int)darray_size(ctx->parent_nodes) - 2 - (int)num < 0) {
        return NULL;
    }
    return darray_item(ctx->parent_nodes, darray_size(ctx->parent_nodes) - 2 - num);
}

i_INLINE_DECL struct ast_node *analyzer_traversal_ctx_get_nth_parent_or_die(
    unsigned int num,
    struct analyzer_traversal_ctx *ctx)
{
    RF_ASSERT((int)darray_size(ctx->parent_nodes) - 2 - (int)num >= 0,
              "Non-existant parent requested");
    return darray_item(ctx->parent_nodes, darray_size(ctx->parent_nodes) - 2 - num);
}

typedef bool (*analyzer_traversal_parents_cb)(const struct ast_node *n, void *user);

/**
 * Traverse parents upwards and run a search callback for each of them.
 * 
 * @param ctx        The analyzer contextx
 * @param cb         The callback function. Should accept a node and an optional
 *                   user argument. If we want to continue searching return false.

 *                   If the callback found what it was looking for return true
 * @param user_arg   An arbitrary argument to pass to the callback
 * @return           Returns @c true if any callback returned succesfully and whatever
 *                   it is we were searching for was found. If nothing is found
 *                   return false
 */
bool analyzer_traversal_ctx_traverse_parents(struct analyzer_traversal_ctx *ctx,
                                             analyzer_traversal_parents_cb cb,
                                             void *user_arg);
/**
 * Finalize the AST of a module after analysis.
 */
bool analyzer_finalize(struct module *m);

// TODO: Change both this, the lexer and the parser macro to something better
#define analyzer_err(mod_, start_, end_, ...)                       \
    do {                                                            \
        i_info_ctx_add_msg((mod_)->front->info,                     \
                           MESSAGE_SEMANTIC_ERROR,                  \
                           (start_),                                \
                           (end_),                                  \
                           __VA_ARGS__);                            \
        /* analyzer_set_semantic_error((mod_)->analyzer);   \ */    \
    } while(0)

#define analyzer_warn(mod_, start_, end_, ...)          \
    do {                                                \
        i_info_ctx_add_msg((mod_)->front->info,         \
                           MESSAGE_SEMANTIC_WARNING,    \
                           (start_),                    \
                           (end_),                      \
                           __VA_ARGS__);                \
    } while(0)

#endif
