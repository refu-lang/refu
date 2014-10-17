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
    return ast_tree_pre_traverse(a->root, cb, user_arg);
}
