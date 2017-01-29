#ifndef LFR_AST_ITERABLE_H
#define LFR_AST_ITERABLE_H

#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>
#include <stdbool.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_iterable_create_identifier(struct ast_node *identifier);

struct ast_node *ast_iterable_create_range(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    int rstart,
    int rstep,
    int rend
);

struct ast_node* ast_iterable_identifier_get(const struct ast_node *it);

#endif
