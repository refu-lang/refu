#include <ast/constants.h>
#include <ast/ast.h>

#include <types/type_elementary.h>

#include <stdint.h>

struct ast_node *ast_constant_create_integer(struct inplocation *loc,
                                             int64_t value)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_CONSTANT, loc);
    if (!ret) {
        return NULL;
    }
    ret->constant.type = CONSTANT_NUMBER_INTEGER;
    ret->constant.value.integer = value;

    return ret;
}

struct ast_node *ast_constant_create_float(struct inplocation *loc,
                                           double value)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_CONSTANT, loc);
    if (!ret) {
        return NULL;
    }
    ret->constant.type = CONSTANT_NUMBER_FLOAT;
    ret->constant.value.floating = value;

    return ret;
}

struct ast_node *ast_constant_create_boolean(const struct inplocation *loc,
                                             bool value)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_CONSTANT, loc);
    if (!ret) {
        return NULL;
    }
    ret->constant.type = CONSTANT_BOOLEAN;
    ret->constant.value.boolean = value;

    return ret;
}
i_INLINE_INS struct ast_node *ast_constant_create_boolean_from_tok(struct token *tok);

const struct type * ast_constant_get_storagetype(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT);
    switch (n->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        if (n->constant.value.integer > INT64_MAX || n->constant.value.integer < INT64_MIN) {
            //TODO: How to handle a literal greater than I64 max?
            RF_ASSERT_OR_CRITICAL(false, "Numeric constant literal value greater"
                                  " that does not fit in 64 bits encountered");
            return NULL;
        }

        // positive constants and zero
        if (n->constant.value.integer > INT32_MAX) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_UINT_64);
        } else if (n->constant.value.integer > INT16_MAX) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_UINT_32);
        } else if (n->constant.value.integer > INT8_MAX) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_UINT_16);
        } else if (n->constant.value.integer >= 0) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_UINT_8);
        // negative constants
        } else if (n->constant.value.integer < INT32_MIN) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_INT_64);
        } else if (n->constant.value.integer < INT16_MIN) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_INT_32);
        } else if (n->constant.value.integer < INT8_MIN) {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_INT_16);
        } else {
            return type_elementary_get_type_constant(ELEMENTARY_TYPE_INT_8);
        }

    case CONSTANT_NUMBER_FLOAT:
        return type_elementary_get_type_constant(ELEMENTARY_TYPE_FLOAT_32);
    case CONSTANT_BOOLEAN:
        return type_elementary_get_type_constant(ELEMENTARY_TYPE_BOOL);
    default:
        RF_ASSERT(false, "Invalid constant type");
        break;
    }
    return NULL;
}

i_INLINE_INS enum constant_type ast_constant_get_type(struct ast_node *n);
i_INLINE_INS bool ast_constant_get_float(struct ast_node *n, double *v);
i_INLINE_INS bool ast_constant_get_integer(struct ast_node *n, int64_t *v);
i_INLINE_INS bool ast_constant_get_bool(const struct ast_node *n);
