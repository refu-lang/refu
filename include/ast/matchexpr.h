#ifndef LFR_AST_MATCHEXPR_H
#define LFR_AST_MATCHEXPR_H

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_matchcase_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *pattern,
                                      struct ast_node *expression);

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id);
#endif
