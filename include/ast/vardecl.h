#ifndef LFR_AST_VARDECL_H
#define LFR_AST_VARDECL_H
struct ast_node;
struct inplocation_mark;

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *desc);
#endif
