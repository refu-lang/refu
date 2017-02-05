#include <ast/iterable.h>
#include <ast/ast.h>

struct ast_node *ast_iterable_create_identifier(struct ast_node *identifier)
{
    struct ast_node *ret = ast_node_create_marks(
        AST_ITERABLE,
        ast_node_startmark(identifier),
        ast_node_endmark(identifier)
    );
    if (!ret) {
        return NULL;
    }
    ret->iterable.type = ITERABLE_COLLECTION;
    ast_node_register_child(ret, identifier, iterable.identifier);

    return ret;
}

struct ast_node *ast_iterable_create_range(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    int rstart,
    int rstep,
    int rend)
{
    struct ast_node *ret = ast_node_create_marks(AST_ITERABLE, start, end);
    if (!ret) {
        return NULL;
    }
    ret->iterable.type = ITERABLE_RANGE;
    ret->iterable.range.start = rstart;
    ret->iterable.range.step = rstep;
    ret->iterable.range.end = rend;

    return ret;
}

int64_t ast_iterable_range_number_of_loops(const struct ast_node *n)
{
    // TODO: When it's impossible to determine in compile time (variable ranges)
    // return -1
    int64_t breadth = abs(ast_iterable_range_end_get(n) - ast_iterable_range_start_get(n));
    int64_t abs_step = abs(ast_iterable_range_step_get(n));
    return abs_step == 0 ? breadth : breadth / abs_step;
}

i_INLINE_INS struct ast_node* ast_iterable_identifier_get(const struct ast_node *it);

i_INLINE_INS enum iterable_type ast_iterable_type_get(const struct ast_node *n);

i_INLINE_INS int64_t ast_iterable_range_start_get(const struct ast_node *n);
i_INLINE_INS int64_t ast_iterable_range_step_get(const struct ast_node *n);
i_INLINE_INS int64_t ast_iterable_range_end_get(const struct ast_node *n);
