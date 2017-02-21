#include <ast/iterable.h>
#include <ast/ast.h>
#include <ast/constants.h>

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
    struct ast_node *start_node,
    struct ast_node *step_node,
    struct ast_node *end_node)
{
    struct ast_node *ret = ast_node_create_marks(
        AST_ITERABLE,
        ast_node_startmark(start_node),
        ast_node_endmark(end_node)
    );

    if (!ret) {
        return NULL;
    }
    ret->iterable.type = ITERABLE_RANGE;
    memset(&ret->iterable.range, 0 , sizeof(struct int_range));
    ret->iterable.range.start = -1;
    ret->iterable.range.step = 1; // default step
    ret->iterable.range.end = -1;


    ast_node_register_child(ret, start_node, iterable.range.start_node);
    if (ast_node_type(start_node) == AST_CONSTANT) {
        if (!ast_constant_get_integer(&start_node->constant, &ret->iterable.range.start)) {
            return NULL;
        }
    }

    if (step_node) {
        ast_node_register_child(ret, step_node, iterable.range.step_node);
        if (ast_node_type(step_node) == AST_CONSTANT) {
            if (!ast_constant_get_integer(&step_node->constant, &ret->iterable.range.step)) {
                return NULL;
            }
        }
    }

    ast_node_register_child(ret, end_node, iterable.range.end_node);
    if (ast_node_type(end_node) == AST_CONSTANT) {
        if (!ast_constant_get_integer(&end_node->constant, &ret->iterable.range.end)) {
            return NULL;
        }
    }

    return ret;
}

bool ast_iterable_range_start_get(const struct ast_node *n, int64_t *ret)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );
    RF_ASSERT(n->iterable.range.start_node, "A start node should exist");

    if (ast_is_constant_integer(n->iterable.range.start_node)) {
        *ret = n->iterable.range.start;
        return true;
    }
    return false;
}

bool ast_iterable_range_step_get(const struct ast_node *n, int64_t *ret)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );

    if (!n->iterable.range.step_node
        || ast_is_constant_integer(n->iterable.range.step_node)) {
        *ret = n->iterable.range.step;
        return true;
    }

    return false;
}

bool ast_iterable_range_end_get(const struct ast_node *n, int64_t *ret)
{
    RF_ASSERT(
        n->type == AST_ITERABLE && n->iterable.type == ITERABLE_RANGE,
        "Illegal ast node type. Expected a range iterable"
    );
    RF_ASSERT(n->iterable.range.end_node, "An end node should exist");

    if (ast_is_constant_integer(n->iterable.range.end_node)) {
        *ret = n->iterable.range.end;
        return true;
    }
    return false;
}

bool ast_iterable_range_compiletime_computable(
    const struct ast_node *n,
    int64_t *start,
    int64_t *step,
    int64_t* end)
{
    if (!ast_iterable_range_start_get(n, start)
        || !ast_iterable_range_step_get(n, step)
        || !ast_iterable_range_end_get(n, end)) {

        // It's impossible to determine the entire range in compile time
        return false;
    }
    return true;
}

int64_t ast_iterable_range_number_of_loops(const struct ast_node *n)
{
    int64_t start;
    int64_t step;
    int64_t end;

    if (!ast_iterable_range_compiletime_computable(n, &start, &step, &end)) {
        // It's impossible to determine the number in compile time
        return -1;
    }

    int64_t breadth = llabs(end - start);
    int64_t abs_step = llabs(step);
    return abs_step == 0 ? breadth : breadth / abs_step;
}

i_INLINE_INS struct ast_node* ast_iterable_identifier_get(const struct ast_node *it);

i_INLINE_INS enum iterable_type ast_iterable_type_get(const struct ast_node *n);


