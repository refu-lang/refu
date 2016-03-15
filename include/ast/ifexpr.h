#ifndef LFR_AST_IF_EXPRESSION_H
#define LFR_AST_IF_EXPRESSION_H

#include <rflib/defs/inline.h>
#include <rflib/utils/sanity.h>

#include <ast/ast.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_condbranch_create(const struct inplocation_mark *start,
                                       const struct inplocation_mark *end,
                                       struct ast_node *cond,
                                       struct ast_node *body);

i_INLINE_DECL struct ast_node *ast_condbranch_condition_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONDITIONAL_BRANCH);
    return n->condbranch.cond;
}

i_INLINE_DECL struct ast_node *ast_condbranch_body_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONDITIONAL_BRANCH);
    return n->condbranch.body;
}

struct ast_node *ast_ifexpr_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *taken_branch,
                                   struct ast_node *fall_through_branch);

i_INLINE_DECL void ast_ifexpr_add_fallthrough_branch(struct ast_node *n,
                                                     struct ast_node *branch)
{
    RF_ASSERT(n->type == AST_BLOCK || n->type == AST_IF_EXPRESSION,
              "Unexpected ast node type");
    ast_node_register_child(n, branch, ifexpr.fallthrough_branch);
}

i_INLINE_DECL struct ast_node *ast_ifexpr_condition_get(const struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.taken_branch->condbranch.cond;
}

i_INLINE_DECL struct ast_node *ast_ifexpr_taken_block_get(const struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.taken_branch->condbranch.body;
}

i_INLINE_DECL struct ast_node *ast_ifexpr_taken_branch_get(const struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.taken_branch;
}

i_INLINE_DECL struct ast_node *ast_ifexpr_fallthrough_branch_get(const struct ast_node *ifexpr)
{
    AST_NODE_ASSERT_TYPE(ifexpr, AST_IF_EXPRESSION);
    return ifexpr->ifexpr.fallthrough_branch;
}

#endif
