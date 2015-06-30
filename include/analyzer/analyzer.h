#ifndef LFR_ANALYZER_H
#define LFR_ANALYZER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>
#include <Data_Structures/intrusive_list.h>
#include <Definitions/inline.h>
#include <Utils/sanity.h>
#include <analyzer/typecheck_matchexpr.h>
#include <analyzer/type_set.h>
#include <ir/rir_types_list.h>

struct parser;
struct inpfile;
struct type;
struct symbol_table;
struct analyzer;
struct front_ctx;
struct module;

#define RECORDS_TABLE_POOL_CHUNK_SIZE 2048

struct  analyzer_traversal_ctx {
    struct module *m;
    //! Remembers the current symbol table during ast traversal
    struct symbol_table *current_st;
    //! Remembers the type of the previous node during the typechecking
    //! Used for assigning types to blocks
    const struct type *last_node_type;
    //! A queue of nodes to remember the current parent of a node during traversal
    struct {darray(struct ast_node*);} parent_nodes;
    //! Pattern matching related data
    struct pattern_matching_ctx matching_ctx;
};

i_INLINE_DECL void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
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
    RF_ASSERT(darray_size(ctx->parent_nodes) - 2 - num >= 0,
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

struct analyzer {
    struct info_ctx *info;

    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;

    //! A set of all types encountered
    struct rf_objset_type *types_set;
    //! A list of all rir types of the file
    struct rir_types_list *rir_types_list;

    /* String tables containing identifiers and string literals found during parsing */
    struct string_table *identifiers_table;
    struct string_table *string_literals_table;

    bool have_semantic_err;

    /* -- options -- */

    /* Warn if there is an implicit type conversion that can cause loss of data */
    #define DEFAULT_WARN_ON_IMPLICIT_CONVERSIONS false
    bool warn_on_implicit_conversions;
};



struct analyzer *analyzer_create(struct info_ctx *info);
bool analyzer_init(struct analyzer *a, struct info_ctx *info);

void analyzer_deinit(struct analyzer *a);
void analyzer_destroy(struct analyzer *a);

/**
 * Determine the dependencies inside a file by checking the imports needed by
 * each module
 */
bool analyzer_determine_dependencies(struct module *m, bool use_stdlib);

/**
 * Analyze a module of a file
 *
 * @param mod                The module to analyze
 *
 * @return                    @c true for success and @c false for failure
 */
bool analyzer_analyze_module(struct module *mod);
/**
 * Finalize the AST of an analyzer after analysis.
 */
bool analyzer_finalize(struct module *m);

/**
 * If existing, retrieve the type and if not existing create the type
 * for ast node @c desc
 *
 * @param m          The module containing the analyzer
 * @param desc       The node whose type to check
 * @param st         The symbol table to check for type existence
 * @param genrdecl   Optional generic delcation that accompanied @c desc.
 *                   Can be NULL.
 * @return           The retrieved or created type, or NULL in error.
 */
struct type *analyzer_get_or_create_type(struct module *m,
                                         const struct ast_node *desc,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl);

// TODO: Properly use the set itself for comparison of already existing types
bool analyzer_types_set_add(struct analyzer *a, struct type *new_type);

i_INLINE_DECL void analyzer_set_semantic_error(struct analyzer *a)
{
    a->have_semantic_err = true;
}

i_INLINE_DECL bool analyzer_has_semantic_error(struct analyzer *a)
{
    return a->have_semantic_err;
}

i_INLINE_DECL bool analyzer_has_semantic_error_reset(struct analyzer *a)
{
    bool ret = a->have_semantic_err;
    a->have_semantic_err = false;
    return ret;
}

// TODO: Change both this, the lexer and the parser macro to something better
#define analyzer_err(mod_, start_, end_, ...) \
    do {                                          \
        i_info_ctx_add_msg((mod_)->analyzer->info,       \
                           MESSAGE_SEMANTIC_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
        analyzer_set_semantic_error((mod_)->analyzer);   \
    } while(0)

#define analyzer_warn(mod_, start_, end_, ...)     \
    do {                                                \
        i_info_ctx_add_msg((mod_)->analyzer->info,           \
                           MESSAGE_SEMANTIC_WARNING,    \
                           (start_),                    \
                           (end_),                      \
                           __VA_ARGS__);                \
    } while(0)

#endif
