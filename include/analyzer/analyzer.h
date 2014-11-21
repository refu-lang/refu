#ifndef LFR_ANALYZER_H
#define LFR_ANALYZER_H

#include <stdbool.h>

#include <Data_Structures/intrusive_list.h>
#include <Definitions/inline.h>
#include <Utils/fixed_memory_pool.h>

#include <analyzer/string_table.h>

struct parser;
struct inpfile;
struct type;
struct symbol_table;
struct analyzer;

struct  analyzer_traversal_ctx {
    struct analyzer *a;
    struct symbol_table *current_st;
};

i_INLINE_DECL void analyzer_traversal_ctx_init(struct analyzer_traversal_ctx *ctx,
                                               struct analyzer *a)
{
    ctx->a = a;
    ctx->current_st = NULL;
}

struct analyzer {
    struct info_ctx *info;
    struct ast_node *root;

    struct rf_fixed_memorypool symbol_table_records_pool;
    struct rf_fixed_memorypool types_pool;

    /* A list of all anonymous types */
    struct RFilist_head anonymous_types;
    /* A list of all types, currently not used due to the symbol tables
     * already having that Information
     */
    struct RFilist_head types;

    struct string_table identifiers_table;
    struct string_table string_literals_table;

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

struct type *analyzer_get_or_create_anonymous_type(struct analyzer *a,
                                                   struct ast_node *desc,
                                                   struct symbol_table *st,
                                                   struct ast_node *genrdecl);

struct inpfile *analyzer_get_file(struct analyzer *a);

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser);

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
#define analyzer_err(analyzer_, start_, end_, ...) \
    do {                                          \
        i_info_ctx_add_msg((analyzer_)->info,       \
                           MESSAGE_SEMANTIC_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
        analyzer_set_semantic_error(analyzer_);         \
    } while(0)

#define analyzer_warn(analyzer_, start_, end_, ...)     \
    do {                                                \
        i_info_ctx_add_msg((analyzer_)->info,           \
                           MESSAGE_SEMANTIC_WARNING,    \
                           (start_),                    \
                           (end_),                      \
                           __VA_ARGS__);                \
    } while(0)

#endif
