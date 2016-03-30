#ifndef LFR_AST_UTILS_H
#define LFR_AST_UTILS_H

#include <stdbool.h>
#include <rflib/datastructs/darray.h>
#include <utils/traversal.h>

struct ast_node;

//! An array of ast nodes
struct arr_ast_nodes {darray(struct ast_node*);};

/**
 * Callback function used by simple ast node traversal
 * @param n           The ast node for which the callback is called
 * @param user_arg    Callback argument
 * @return            true in success and false in failure.
 */
typedef bool (*ast_node_cb) (struct ast_node *n, void *user_arg);

/**
 * Callback function used by no-stop ast node traversal
 * @param n           The ast node for which the callback is called
 * @param user_arg    Callback argument
 * @return            @see ast_traversal_cb_res
 */
typedef enum traversal_cb_res (*ast_node_nostop_cb) (struct ast_node *n, void *user_arg);

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
 * Post-order traversal of the AST tree
 *
 * @param n              The node to traverse
 * @param cb             A callback to be executed at tree traversal for each
 *                       node in post-order. i.e: Going from the bottom up
 * @param user_arg       Argument passed to the callback
 */
bool ast_post_traverse_tree(struct ast_node *n,
                            ast_node_cb cb,
                            void *user_arg);

/**
 * Traversal of the AST tree
 *
 * @used_by  Analyzer pass 1, Symbol table creation
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
 *
 * @return               true for succesfull traversal of the tree and false if
 *                       there was an error. Note that unlike @see ast_traverse_tree_nostop
 *                       a false return value would mean that it stopped at the
 *                       first failed callback.
 */
bool ast_traverse_tree(struct ast_node *n,
                       ast_node_cb pre_cb,
                       void *pre_user_arg,
                       ast_node_cb post_cb,
                       void *post_user_arg);

/**
 * Traversal of the AST tree without stopping at errors at @c post_cb
 *
 * Identical to @see ast_traverse_tree except for the fact that it offers more choices
 * to the post_order traversal callback.
 *
 * @used_by  Typechecking
 *
 * @return              AST_TRAVERSAL_OK if all went fine with all the callbacks
 *                      of the traversal and other values of @ref ast_traversal_cb_res
 *                      in case of errors
 */
enum traversal_cb_res ast_traverse_tree_nostop_post_cb(struct ast_node *n,
                                                       ast_node_cb pre_cb,
                                                       void *pre_user_arg,
                                                       ast_node_nostop_cb post_cb,
                                                       void *post_user_arg);



typedef bool (*exprlist_cb) (struct ast_node *n, void *user_arg);
/**
 * In a big comma-separated expression iterate all sub-expressions
 *
 * Should be called only after typechecking
 *
 * @param n            The comma separated expression to iterate
 * @param cb           The callback to execute for each sub expression
 * @param user         The extra argument to provide to the callback
 */
bool ast_foreach_expr(struct ast_node *n, exprlist_cb cb, void *user);
#endif
