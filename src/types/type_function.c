#include <types/type_function.h>

i_INLINE_INS struct type *type_function_get_argtype(const struct type *t);
i_INLINE_INS struct type *type_function_get_rettype(const struct type *t);

static inline bool type_is_product_op(const struct type *t)
{
    return t->category == TYPE_CATEGORY_ANONYMOUS &&
        t->anonymous.is_operator &&
        t->anonymous.op.type == TYPEOP_PRODUCT;
}

static const struct type *do_type_function_get_argtype_n(const struct type_operator *op,
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

    //it should be a product operator, since arguments should be comma separated
    if (!type_is_product_op(t)) {
        return NULL;
    }

    // continue recursion
    return do_type_function_get_argtype_n(&t->anonymous.op, n - 1);
}

const struct type *type_function_get_argtype_n(const struct type *t, unsigned int n)
{
    // special case of 1 argument only
    if (n == 0 && t->category == TYPE_CATEGORY_LEAF) {
        return t->anonymous.op.left->leaf.type;
    }

    // else it should be a product operator, since arguments should be comma separated
    if (!type_is_product_op(t)) {
        return NULL;
    }

    return do_type_function_get_argtype_n(&t->anonymous.op, n);
}
