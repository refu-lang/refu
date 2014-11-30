#ifndef LFR_AST_UTILS_H
#define LFR_AST_UTILS_H

#include <stdbool.h>

struct ast_node;


typedef bool (*ast_node_cb) (struct ast_node *n, void *user_arg);

/**
 * Pre-order traversal of the AST tree
 *
 * @param n              The node to traverse
 * @param cb             A callback to be executed at tree traversal for each
 *                       node in pre-order. i.e: When the node is visited for
 *                       the first time
 * @param user_arg       Argument passed to the callback
 */
bool ast_pre_traverse_tree(struct ast_node *n,
                           ast_node_cb cb,
                           void *user_arg);

/**
 * Traversal of the AST tree
 *
 * @param n              The node to traverse
 * @param pre_cb         A callback to be executed at tree traversal for each
 *                       node in pre-order. i.e: When the node is visited for
 *                       the first time
 * @param pre_user_arg   Argument passed to the preorder callback
 * @param post_cb        A callback to be executed at tree traversal for each
 *                       node in post-order. i.e: when its whole subtree is
 *                       abandoned and we are going back to its parent.
 * @param post_user_arg  Argument passed to the postorder callback
 */
bool ast_traverse_tree(struct ast_node *n,
                       ast_node_cb pre_cb,
                       void *pre_user_arg,
                       ast_node_cb post_cb,
                       void *post_user_arg);

#endif
