#ifndef LFR_AST_ITERABLE_H
#define LFR_AST_ITERABLE_H

#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>
#include <ast/ast.h>
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

/**
 * Return (if possible) the number of loops the range will create
 * @param n The iterable ast node
 * @return The number of loops the range will create or -1 if it's not possible
 *         to determine in compile time.
 */
int64_t ast_iterable_range_number_of_loops(const struct ast_node *n);

i_INLINE_DECL struct ast_node* ast_iterable_identifier_get(const struct ast_node *n)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_COLLECTION,
        "Illegal ast node type. Expected a collection iterable"
    );
    return n->iterable.identifier;
}

i_INLINE_DECL enum iterable_type ast_iterable_type_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_ITERABLE);
    return n->iterable.type;
}

i_INLINE_DECL int64_t ast_iterable_range_start_get(const struct ast_node *n)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );
    return n->iterable.range.start;
}

i_INLINE_DECL int64_t ast_iterable_range_step_get(const struct ast_node *n)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );
    return n->iterable.range.step;
}

i_INLINE_DECL int64_t ast_iterable_range_end_get(const struct ast_node *n)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );
    return n->iterable.range.end;
}

#endif
