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

struct ast_node *ast_ifexpr_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *taken_branch,
                                   struct ast_node *fall_through_branch);

i_INLINE_DECL void ast_ifexpr_add_fall_through_branch(struct ast_node *n,
                                                      struct ast_node *branch)
{
    RF_ASSERT(branch->type == AST_CONDITIONAL_BRANCH);
    ast_node_register_child(n, branch, ifexpr.fall_through_branch);
}

i_INLINE_DECL void ast_ifexpr_add_elif_branch(struct ast_node *n,
                                              struct ast_node *branch)
{
    RF_ASSERT(branch->type == AST_CONDITIONAL_BRANCH);
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
        RF_ASSERT(false); // should never happen
        RF_CRITICAL("Illegal token type for if expression branch addition");
    }
}
#endif
