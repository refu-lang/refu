#ifndef LFR_AST_TYPECLASS_H
#define LFR_AST_TYPECLASS_H

#include <ast/ast.h>

struct ast_node *ast_typeclass_create(struct inplocation_mark *start,
                                      struct inplocation_mark *end,
                                      struct ast_node *name,
                                      struct ast_node *genr);
#endif
