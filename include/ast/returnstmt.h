#ifndef LFR_AST_RETURNSTMT_H
#define LFR_AST_RETURNSTMT_H

#include <ast/ast.h>

struct ast_node *ast_returnstmt_create(struct inplocation_mark *start,
                                       struct inplocation_mark *end,
                                       struct ast_node *expr);
#endif
