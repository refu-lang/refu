#include <ast/ast_utils.h>

#include <ast/ast.h>
#include <ast/operators.h>

bool ast_pre_traverse_tree(struct ast_node *n,
                           ast_node_cb cb,
                           void *user_arg)
{
    if (!cb(n, user_arg)) {
        return false;
    }

    struct ast_node **child;
    darray_foreach(child, n->children) {
        if (!ast_pre_traverse_tree(*child, cb, user_arg)) {
            return false;
        }
    }

    return true;
}

bool ast_post_traverse_tree(struct ast_node *n,
                            ast_node_cb cb,
                            void *user_arg)
{
    struct ast_node **child;
    darray_foreach(child, n->children) {
        if (!ast_post_traverse_tree(*child, cb, user_arg)) {
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
    if (!pre_cb(n, pre_user_arg)) {
        return false;
    }

    struct ast_node **child;
    darray_foreach(child, n->children) {
        if (!ast_traverse_tree(
                *child,
                pre_cb,
                pre_user_arg,
                post_cb,
                post_user_arg))
        {
            return false;
        }
    }

    if (!post_cb(n, post_user_arg)) {
        return false;
    }

    return true;
}

enum traversal_cb_res ast_traverse_tree_nostop_post_cb(
    struct ast_node *n,
    ast_node_cb pre_cb,
    void *pre_user_arg,
    ast_node_nostop_cb post_cb,
    void *post_user_arg)
{
    enum traversal_cb_res rc;
    enum traversal_cb_res ret = TRAVERSAL_CB_OK;

    if (!pre_cb(n, pre_user_arg)) {
        return TRAVERSAL_CB_ERROR;
    }

    struct ast_node **child;
    darray_foreach(child, n->children) {
        rc = ast_traverse_tree_nostop_post_cb(
            *child,
            pre_cb,
            pre_user_arg,
            post_cb,
            post_user_arg
        );
        if (rc == TRAVERSAL_CB_FATAL_ERROR) {
            return rc;
        } else if (rc == TRAVERSAL_CB_ERROR) {
            // keep the fact we errored for return but keep traversing
            ret = rc;
        }
    }

    rc = post_cb(n, post_user_arg);
    if (rc == TRAVERSAL_CB_FATAL_ERROR) {
        return rc;
    } else if (rc == TRAVERSAL_CB_ERROR) {
        // keep the fact we errored for return but keep traversing
        ret = rc;
    }

    return ret;
}

bool ast_foreach_expr(struct ast_node *n, exprlist_cb cb, void *user)
{
    if (ast_node_is_specific_binaryop(n, BINARYOP_COMMA)) {
        if (!ast_foreach_expr(ast_binaryop_left(n), cb, user)) {
            return false;
        }
        if (!ast_foreach_expr(ast_binaryop_right(n), cb, user)) {
            return false;
        }
        return true;
    }
    return cb(n, user);
}
