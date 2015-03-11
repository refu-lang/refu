#ifndef LFR_AST_TYPECLASS_H
#define LFR_AST_TYPECLASS_H

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_typeclass_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *name,
                                      struct ast_node *genr);

struct ast_node *ast_typeinstance_create(const struct inplocation_mark *start,
                                         const struct inplocation_mark *end,
                                         struct ast_node *class_name,
                                         struct ast_node *type_name,
                                         struct ast_node *genr);
#endif
