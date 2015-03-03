#include <ast/ast.h>
#include <ast/ifexpr.h>

struct ast_node *ast_condbranch_create(struct inplocation_mark *start,
                                       struct inplocation_mark *end,
                                       struct ast_node *cond,
                                       struct ast_node *body)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_CONDITIONAL_BRANCH, start, end);
    if (!ret) {
        return NULL;
    }
   
    ast_node_register_child(ret, cond, condbranch.cond);
    ast_node_register_child(ret, body, condbranch.body);

    return ret;
}

i_INLINE_INS struct ast_node *ast_condbranch_condition_get(struct ast_node *n);
i_INLINE_INS struct ast_node *ast_condbranch_body_get(struct ast_node *n);

struct ast_node *ast_ifexpr_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *taken_branch,
                                   struct ast_node *fall_through_branch)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_IF_EXPRESSION, start, end);
    if (!ret) {
        return NULL;
    }
   
    ast_node_register_child(ret, taken_branch, ifexpr.taken_branch);
    ast_node_register_child(ret, fall_through_branch, ifexpr.fall_through_branch);

    return ret;    
}

i_INLINE_INS void ast_ifexpr_add_fall_through_branch(struct ast_node *n,
                                                     struct ast_node *branch);
i_INLINE_INS void ast_ifexpr_add_elif_branch(struct ast_node *n,
                                             struct ast_node *branch);
i_INLINE_INS void ast_ifexpr_add_branch(struct ast_node *n,
                                        struct ast_node *branch,
                                        enum token_type type);

i_INLINE_INS struct ast_node *ast_ifexpr_taken_branch_get(struct ast_node *ifexpr);
i_INLINE_INS struct ast_node *ast_ifexpr_fallthrough_branch_get(struct ast_node *ifexpr);

size_t ast_ifexpr_branches_num_get(struct ast_node *ifexpr)
{
    size_t num = 0;
    struct ast_node *c;
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    rf_ilist_for_each(&ifexpr->children, c, lh) {
        ++num;
    }
    return num;
}
