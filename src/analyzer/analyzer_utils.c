#include "analyzer_utils.h"

#include <analyzer/analyzer.h>
#include <ast/ast.h>

static bool ast_tree_pre_traverse(struct ast_node *n,
                                  analyzer_tree_node_cb cb,
                                  void *user_arg)
{
    struct ast_node *child;

    if (!cb(n, user_arg)) {
        return false;
    }

    rf_ilist_for_each(&n->children, child, lh) {
        if (!ast_tree_pre_traverse(child, cb, user_arg)) {
            return false;
        }
    }

    return true;
}

bool analyzer_pre_traverse_tree(struct analyzer *a,
                                analyzer_tree_node_cb cb,
                                void *user_arg)
{
    RF_ASSERT(cb, "callback function not given");
    return ast_tree_pre_traverse(a->root, cb, user_arg);
}

static bool ast_tree_traverse(struct ast_node *n,
                              analyzer_tree_node_cb pre_cb,
                              void *pre_user_arg,
                              analyzer_tree_node_cb post_cb,
                              void *post_user_arg)
{
    struct ast_node *child;
    if (!pre_cb(n, pre_user_arg)) {
        return false;
    }

    rf_ilist_for_each(&n->children, child, lh) {
        if (!ast_tree_traverse(child, pre_cb, pre_user_arg,
                               post_cb, post_user_arg)) {
            return false;
        }
    }

    if (!post_cb(n, post_user_arg)) {
        return false;
    }

    return true;
}

bool analyzer_traverse_tree(struct analyzer *a,
                            analyzer_tree_node_cb pre_cb,
                            void *pre_user_arg,
                            analyzer_tree_node_cb post_cb,
                            void *post_user_arg)
{
    RF_ASSERT(pre_cb != NULL && post_cb != NULL, "callback function not given");
    return ast_tree_traverse(a->root, pre_cb, pre_user_arg, post_cb, post_user_arg);
}
