#include <ast/constant_num.h>
#include <ast/ast.h>

struct ast_node *ast_constantnum_create_integer(struct inplocation *loc,
                                                uint64_t value)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_CONSTANT_NUMBER, loc);
    if (!ret) {
        return NULL;
    }
    ret->constantnum.type = CONSTANT_NUMBER_INTEGER;
    ret->constantnum.value.integer = value;

    return ret;
}

struct ast_node *ast_constantnum_create_float(struct inplocation *loc,
                                              double value)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_CONSTANT_NUMBER, loc);
    if (!ret) {
        return NULL;
    }
    ret->constantnum.type = CONSTANT_NUMBER_FLOAT;
    ret->constantnum.value.floating = value;

    return ret;
}

i_INLINE_INS bool ast_constantnum_get_float(struct ast_node *n, double *v);
i_INLINE_INS bool ast_constantnum_get_integer(struct ast_node *n, uint64_t *v);
