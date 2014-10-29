#ifndef LFR_ANALYZER_H
#define LFR_ANALYZER_H

#include <stdbool.h>

#include <Definitions/inline.h>
#include <Utils/fixed_memory_pool.h>

struct parser;
struct inpfile;

struct analyzer {
    struct info_ctx *info;
    struct ast_node *root;

    struct rf_fixed_memorypool symbol_table_records_pool;
    bool have_semantic_err;
};

struct analyzer *analyzer_create(struct info_ctx *info);
bool analyzer_init(struct analyzer *a, struct info_ctx *info);

void analyzer_deinit(struct analyzer *a);
void analyzer_destroy(struct analyzer *a);

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

#endif
