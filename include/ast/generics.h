#ifndef LFR_AST_GENERICS_H
#define LFR_AST_GENERICS_H

#include <ast/generics_decls.h>

struct inplocation_mark;
struct ast_node *ast_genrtype_create(struct ast_node *type, struct ast_node *id);

struct ast_node *ast_genrdecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end);

struct ast_node *ast_genrattr_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end);
#endif
