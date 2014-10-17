#ifndef LFR_ANALYZER_UTILS_H
#define LFR_ANALYZER_UTILS_H

#include <stdbool.h>

struct analyzer;
struct ast_node;


typedef bool (*analyzer_tree_node_cb) (struct ast_node *n, void *user_arg);

/**
 * Pre-order traversal of the AST tree during the analyzing phase
 */
bool analyzer_pre_traverse_tree(struct analyzer *a,
                                analyzer_tree_node_cb cb,
                                void *user_arg);

#endif
