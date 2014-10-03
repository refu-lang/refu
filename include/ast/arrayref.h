#ifndef LFR_AST_ARRAY_REFERENCE_H
#define LFR_AST_ARRAY_REFERENCE_H

#include <ast/ast.h>

struct ast_node *ast_arrayref_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *expr);
#endif
