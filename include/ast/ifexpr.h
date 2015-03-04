#ifndef LFR_AST_IF_EXPRESSION_H
#define LFR_AST_IF_EXPRESSION_H

#include <ast/ast.h>

#include <Definitions/inline.h>
#include <Utils/sanity.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_condbranch_create(struct inplocation_mark *start,
                                       struct inplocation_mark *end,
                                       struct ast_node *cond,
                                       struct ast_node *body);

i_INLINE_DECL struct ast_node *ast_condbranch_condition_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONDITIONAL_BRANCH);
    return n->condbranch.cond;
}

i_INLINE_DECL struct ast_node *ast_condbranch_body_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONDITIONAL_BRANCH);
    return n->condbranch.body;
}

struct ast_node *ast_ifexpr_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *taken_branch,
                                   struct ast_node *fall_through_branch);

i_INLINE_DECL void ast_ifexpr_add_fall_through_branch(struct ast_node *n,
                                                      struct ast_node *branch)
{
    AST_NODE_ASSERT_TYPE(branch, AST_BLOCK);
    ast_node_register_child(n, branch, ifexpr.fall_through_branch);
}

i_INLINE_DECL void ast_ifexpr_add_elif_branch(struct ast_node *n,
                                              struct ast_node *branch)
{
    AST_NODE_ASSERT_TYPE(branch, AST_CONDITIONAL_BRANCH);
    ast_node_add_child(n, branch);
}

i_INLINE_DECL void ast_ifexpr_add_branch(struct ast_node *n,
                                         struct ast_node *branch,
                                         enum token_type type)
{
    if (type == TOKEN_KW_ELIF) {
        ast_ifexpr_add_elif_branch(n, branch);
    } else if (type == TOKEN_KW_ELSE) {
        ast_ifexpr_add_fall_through_branch(n, branch);
    } else {
        // should never happen
        RF_ASSERT_OR_CRITICAL(
            false,
            "Illegal token type for if expression branch addition");
    }
}

i_INLINE_DECL struct ast_node *ast_ifexpr_taken_branch_get(struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.taken_branch;
}

i_INLINE_DECL struct ast_node *ast_ifexpr_fallthrough_branch_get(struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.fall_through_branch;
}

size_t ast_ifexpr_branches_num_get(struct ast_node *ifexpr);

#define ast_ifexpr_branches_for_each(ifexpr_, branch_)      \
    AST_NODE_ASSERT_TYPE(ifexpr_, AST_IF_EXPRESSION);       \
    rf_ilist_for_each(&ifexpr_->children, branch_, lh)


#endif
