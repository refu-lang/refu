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
    ast_constant_init_int(&ret->constant, value);
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
    ast_constant_init_float(&ret->constant, value);
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
    ast_constant_init_bool(&ret->constant, value);
    return ret;
}
i_INLINE_INS struct ast_node *ast_constant_create_boolean_from_tok(struct token *tok);
i_INLINE_INS void ast_constant_init_int(struct ast_constant *c, int64_t n);
i_INLINE_INS void ast_constant_init_float(struct ast_constant *c, double n);
i_INLINE_INS void ast_constant_init_bool(struct ast_constant *c, bool n);

const struct RFstring *ast_constant_string(const struct ast_constant *c)
{
    const struct RFstring *ret;
    switch(c->type) {
    case CONSTANT_NUMBER_FLOAT:
        ret = RFS("%f", c->value.floating);
        break;
    case CONSTANT_NUMBER_INTEGER:
        ret = RFS("%"PRId64, c->value.integer);
        break;
    case CONSTANT_BOOLEAN:
        ret = RFS("%s", c->value.boolean ? "true" : "false");
        break;
    }
    return ret;
}

const struct type * ast_constant_get_storagetype(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_CONSTANT);
    switch (n->constant.type) {
    case CONSTANT_NUMBER_INTEGER:
        //TODO: How to handle a literal greater than I64 max?
        RF_ASSERT_OR_CRITICAL(n->constant.value.integer <= INT64_MAX ||
                              n->constant.value.integer >= INT64_MIN,
                              return NULL,
                              "Numeric constant literal value greater"
                              " that does not fit in 64 bits encountered");

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
        RF_CRITICAL_FAIL("Invalid constant type");
        break;
    }
    return NULL;
}

i_INLINE_INS enum constant_type ast_constant_get_type(const struct ast_constant *n);
i_INLINE_INS bool ast_constant_get_float(const struct ast_constant *n, double *v);
i_INLINE_INS bool ast_constant_get_integer(const struct ast_constant *n, int64_t *v);
i_INLINE_INS bool ast_constant_get_bool(const struct ast_constant *n);
