#ifndef LFR_ANALYZER_H
#define LFR_ANALYZER_H

#include <stdbool.h>

struct parser;

struct analyzer {
    struct info_ctx *info;
    struct ast_node *root;
};

struct analyzer *analyzer_create(struct info_ctx *info);
bool analyzer_init(struct analyzer *a, struct info_ctx *info);

void analyzer_deinit(struct analyzer *a);
void analyzer_destroy(struct analyzer *a);

bool analyzer_analyze_file(struct analyzer *a, struct parser *parser);
#endif
