#ifndef LFR_AST_IF_EXPRESSION_H
#define LFR_AST_IF_EXPRESSION_H

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
#endif
