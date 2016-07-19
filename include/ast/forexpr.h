#ifndef LFR_AST_FOR_EXPRESSION_H
#define LFR_AST_FOR_EXPRESSION_H

#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>

#include <ast/ast.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_forexpr_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *loopvar,
    struct ast_node *iterable,
    struct ast_node *body
);


#endif
