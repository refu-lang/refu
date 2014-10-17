#ifndef LFR_AST_CONSTANT_NUMBER_H
#define LFR_AST_CONSTANT_NUMBER_H

#include <stdbool.h>
#include <stdint.h>
#include <Definitions/inline.h>

struct ast_node;
struct inplocation;

struct ast_node *ast_constantnum_create_integer(struct inplocation *loc,
                                                uint64_t value);
struct ast_node *ast_constantnum_create_float(struct inplocation *loc,
                                              double value);


#include <ast/ast.h>

i_INLINE_DECL enum constant_type ast_constantnum_get_type(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT_NUMBER);
    return n->constantnum.type;
}

i_INLINE_DECL bool ast_constantnum_get_float(struct ast_node *n, double *v)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT_NUMBER);

    if (n->constantnum.type != CONSTANT_NUMBER_FLOAT) {
        return false;
    }
    *v = n->constantnum.value.floating;

    return true;
}

i_INLINE_DECL bool ast_constantnum_get_integer(struct ast_node *n, uint64_t *v)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT_NUMBER);

    if (n->constantnum.type != CONSTANT_NUMBER_INTEGER) {
        return false;
    }
    *v = n->constantnum.value.integer;

    return true;
}

#endif
