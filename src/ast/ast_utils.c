#include <ast/ast_utils.h>

#include <ast/ast.h>

bool ast_pre_traverse_tree(struct ast_node *n,
                           ast_node_cb cb,
                           void *user_arg)
{
    struct ast_node *child;

    if (!cb(n, user_arg)) {
        return false;
    }

    rf_ilist_for_each(&n->children, child, lh) {
        if (!ast_pre_traverse_tree(child, cb, user_arg)) {
            return false;
        }
    }

    return true;
}

bool ast_post_traverse_tree(struct ast_node *n,
                            ast_node_cb cb,
                            void *user_arg)
{
    struct ast_node *child;

    rf_ilist_for_each(&n->children, child, lh) {
        if (!ast_post_traverse_tree(child, cb, user_arg)) {
            return false;
        }
    }

    if (!cb(n, user_arg)) {
        return false;
    }

    return true;
}

bool ast_traverse_tree(struct ast_node *n,
                       ast_node_cb pre_cb,
                       void *pre_user_arg,
                       ast_node_cb post_cb,
                       void *post_user_arg)
{
    struct ast_node *child;
    if (!pre_cb(n, pre_user_arg)) {
        return false;
    }

    rf_ilist_for_each(&n->children, child, lh) {
        if (!ast_traverse_tree(child, pre_cb, pre_user_arg,
                               post_cb, post_user_arg)) {
            return false;
        }
    }

    if (!post_cb(n, post_user_arg)) {
        return false;
    }

    return true;
}

enum ast_traversal_cb_res ast_traverse_tree_nostop_post_cb(struct ast_node *n,
                                                           ast_node_cb pre_cb,
                                                           void *pre_user_arg,
                                                           ast_node_nostop_cb post_cb,
                                                           void *post_user_arg)
{
    struct ast_node *child;
    enum ast_traversal_cb_res rc;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_OK;

    if (!pre_cb(n, pre_user_arg)) {
        return false;
    }

    rf_ilist_for_each(&n->children, child, lh) {
        rc = ast_traverse_tree_nostop_post_cb(child, pre_cb, pre_user_arg, post_cb, post_user_arg);
        if (rc == AST_TRAVERSAL_FATAL_ERROR) {
            return rc;
        } else if (rc == AST_TRAVERSAL_ERROR) {
            ret = rc;
        }
    }

    rc = post_cb(n, post_user_arg);
    if (rc == AST_TRAVERSAL_FATAL_ERROR) {
        return rc;
    } else if (rc == AST_TRAVERSAL_ERROR) {
        ret = rc;
    }

    return ret;
}
