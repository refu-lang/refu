#include <types/type_function.h>

#include <module.h>
#include <types/type.h>
#include <String/rf_str_core.h>

i_INLINE_INS bool type_is_function(const struct type *t);
i_INLINE_INS bool type_is_callable(const struct type *t);
i_INLINE_INS struct type *type_callable_get_argtype(const struct type *t);
i_INLINE_INS void type_function_set_argtype(struct type *t, struct type *other);
i_INLINE_INS struct type *type_function_get_rettype(const struct type *t);
i_INLINE_INS struct type *type_callable_get_rettype(const struct type *t);
i_INLINE_INS void type_function_set_rettype(struct type *t, struct type *other);

static const struct RFstring s_function_ = RF_STRING_STATIC_INIT("function");
static const struct RFstring s_ctor_ = RF_STRING_STATIC_INIT("constructor");

struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    return t->operator.left;
}

const struct RFstring *type_callable_category_str(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return &s_function_;
    }
    return &s_ctor_;
}

static inline bool type_is_product_op(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_PRODUCT;
}

static const struct type *do_type_fnargs_get_argtype_n(const struct type_operator *op,
                                                       unsigned int n)
{
    const struct type *t;
    if (n == 0) {
        t = op->left;
        // if it's a leaf don't take the identifier
        if (t->category == TYPE_CATEGORY_LEAF) {
            return t->leaf.type;
        }
        return NULL;
    } else {
        t = op->right;
        // special case of only 2 arguments
        if (n == 1 && t->category != TYPE_CATEGORY_LEAF) {
            return NULL;
        }
        return t->leaf.type;
    }
    RF_ASSERT_OR_CRITICAL(type_is_product_op(t),
                          return NULL,
                          "function call arguments can only be comma separated");

    // continue recursion
    return do_type_fnargs_get_argtype_n(&t->operator, n - 1);
}

void type_function_init(struct type *t, struct type *arg_type, struct type *ret_type)
{
    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = TYPEOP_IMPLICATION;

    type_function_set_argtype(t, arg_type);
    type_function_set_rettype(t, ret_type);
}

const struct type *type_fnargs_get_argtype_n(const struct type *t, unsigned int n)
{
    // special case of 1 argument only
    if (n == 0 && t->category == TYPE_CATEGORY_LEAF) {
        return t->operator.left->leaf.type;
    }
    RF_ASSERT_OR_CRITICAL(type_is_product_op(t),
                          return NULL,
                          "function call arguments can only be comma separated");

    return do_type_fnargs_get_argtype_n(&t->operator, n);
}
