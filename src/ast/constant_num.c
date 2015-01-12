#include <ast/constant_num.h>
#include <ast/ast.h>

#include <types/type_builtin.h>

#include <stdint.h>

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

const struct type * ast_constantnum_get_storagetype(struct ast_node *n)
{
    enum constant_type ctype = ast_constantnum_get_type(n);
    if (ctype == CONSTANT_NUMBER_INTEGER) {
        if (n->constantnum.value.integer > UINT64_MAX) {
            //TODO: How to handle a literal greater than U64 max?
            RF_ASSERT_OR_CRITICAL(false, "Numeric constant literal value greater"
                                  " than UINT64_MAX encountered");
            return NULL;
        }

        if (n->constantnum.value.integer > UINT32_MAX) {
            return type_builtin_get_type(BUILTIN_UINT_64);
        }

        if (n->constantnum.value.integer > UINT16_MAX) {
            return type_builtin_get_type(BUILTIN_UINT_32);
        }

        if (n->constantnum.value.integer > UINT8_MAX) {
            return type_builtin_get_type(BUILTIN_UINT_16);
        }

        return type_builtin_get_type(BUILTIN_UINT_8);
    }

    // else it's a float literal (simple for now)
    return type_builtin_get_type(BUILTIN_FLOAT_64);
}

i_INLINE_INS enum constant_type ast_constantnum_get_type(struct ast_node *n);
i_INLINE_INS bool ast_constantnum_get_float(struct ast_node *n, double *v);
i_INLINE_INS bool ast_constantnum_get_integer(struct ast_node *n, uint64_t *v);
